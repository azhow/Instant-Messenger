#include "CServer.h"

#include <unistd.h>
#include <strings.h>
#include <exception>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <mutex>
#include <memory>
#include <netinet/tcp.h>

std::mutex g_diskMutex{ std::mutex() };
std::mutex g_loginMutex{ std::mutex() };
std::mutex g_broadcastMutex{ std::mutex() };

CServer::CServer(std::size_t numberOfMsgToRetrieve, std::uint16_t port) :
    m_nMessagesToRetrieve(numberOfMsgToRetrieve),
    m_port(port),
    m_groupFolderPath{ std::filesystem::current_path() },
    m_users{}
{
    // Groups folder
    m_groupFolderPath.append("Groups");
    // If group folder does not exist, we should create it
    if (!std::filesystem::exists(m_groupFolderPath))
    {
        if (!std::filesystem::create_directory(m_groupFolderPath))
        {
            throw std::runtime_error("Could not create group folder");
        }
    }

    // Socket file descriptor
    m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (m_serverSocket == -1)
    {
        // Error
        throw std::runtime_error("Could not create server socket");
    }

    // Server address struct
    struct sockaddr_in servAddr;

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(m_port);
    servAddr.sin_addr.s_addr = INADDR_ANY;
    // Zero fill array
    bzero(&(servAddr.sin_zero), 8);

    // Option value for option level
    const int cOptionValue{ 1 };

    // Keep alive idle time before first check and keep alive time in seconds
    const int cKeepAliveTime{ 60 };

    // Set server socket to allow immediate reuse of the port
    if (setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &cOptionValue, sizeof(int)) == 0)
    {
        // Set keep alive
        if (setsockopt(m_serverSocket, SOL_SOCKET, SO_KEEPALIVE, &cOptionValue, sizeof(int)) == 0)
        {
            // Set keep timer
            if (setsockopt(m_serverSocket, SOL_TCP, TCP_KEEPIDLE, &cKeepAliveTime, sizeof(int)) == 0)
            {
                // Set keep alive
                if (setsockopt(m_serverSocket, SOL_TCP, TCP_KEEPINTVL, &cKeepAliveTime, sizeof(int)) == 0)
                {
                    // Bind socket to address
                    if (bind(m_serverSocket, (struct sockaddr*)&servAddr, sizeof(servAddr)) != 0)
                    {
                        // Error
                        throw std::runtime_error("Could not bind server socket");
                    }
                }
                else
                {
                    // Error
                    throw std::runtime_error("Could not set keep alive timer server socket option");
                }
            }
            else
            {
                // Error
                throw std::runtime_error("Could not set idle time server socket option");
            }
        }
        else
        {
            // Error
            throw std::runtime_error("Could not set keep alive server socket option");
        }
    }
    else
    {
        // Error
        throw std::runtime_error("Could not set reuse server socket option");
    }

    std::cout << "[INFO] Starting server in port: " + std::to_string(m_port) + " address: "
        + std::to_string(INADDR_ANY) << std::endl;
}

CServer::~CServer()
{
    // Close server socket
    close(m_serverSocket);
}

void
CServer::waitForConnections()
{
    // The size of the queue of waiting connections
    const int cConnectionQueueSize{ 5 };

    // Waits for new connections and creates handler threads
    while (true)
    {
        // Listens for new connections
        if (listen(m_serverSocket, cConnectionQueueSize) != 0)
        {
            // Error
            throw std::runtime_error("Could not listen for new connections");
        }

        // Client address to be read when accepting connection
        struct sockaddr_in clientAddr;
        // Client socket length
        socklen_t clientLength{ sizeof(struct sockaddr_in) };
        // Client socket file descriptor
        int clientSocket = accept(m_serverSocket, (struct sockaddr*)&clientAddr, &clientLength);
        // Check if client socket is valid
        if (clientSocket == -1)
        {
            // Error
            throw std::runtime_error("Could not accept incoming connection");
        }

        // Create new handler thread for connection
        m_handlerThreads->push_back(std::thread(&CServer::handleClientConnection, this, clientSocket));
    }
}

void
CServer::handleClientConnection(int clientSocket)
{
    try
    {
        // Current user
        const auto& [currentUser, currentGroupID]{ login(clientSocket) };
        // Has the connection been closed?
        bool isConnectionClosed{ false };

        // Maybe we need to create a close message or timeout for the socket
        while (!isConnectionClosed)
        {
            // Listen for incoming messages from user
            // Read message header
            const CMessage cReceivedMessage{ CMessage::readMessageFromSocket(clientSocket, isConnectionClosed) };

            if (!isConnectionClosed)
            {
                m_messageBuffer->push_back(cReceivedMessage);

                broadcastMessage(clientSocket, cReceivedMessage);
            }
            else
            {
                // Send message of user logoff
                const CMessage cBroadcastMessage{ "Server", currentGroupID,
                    currentUser->getUserID() + " has disconnected" };

                // Print info on server
                std::cout << "[INFO] " + currentUser->getUserID() + " has disconnected" << std::endl;

                broadcastMessage(clientSocket, cBroadcastMessage);
            }
        }

        // Remove client from group
        removeFromGroup(currentUser, currentGroupID);

        // Close client socket
        close(clientSocket);
    }
    catch (const std::exception& exception)
    {
        // Close client socket
        close(clientSocket);
    }

    syncToDisk();
}

std::list<CMessage>
CServer::retrieveLastNMessages(const std::string& groupID) const
{
    // Last messages from group
    std::list<CMessage> lastNMessagesFromGroup{};

    // Group file
    std::filesystem::path groupFilePath = m_groupFolderPath;
    groupFilePath.append(groupID + ".msg");

    // Search in the disk if there's an existing group file
    if (std::filesystem::is_regular_file(groupFilePath))
    {
        // Group file
        std::ifstream fileStream(groupFilePath, std::ios::out | std::ios::binary);

        // Check if successfully opened the file
        if (fileStream.good())
        {
            // Read all messages from file
            while (!fileStream.eof())
            {
                lastNMessagesFromGroup.push_back(CMessage::readMessageFromDisk(fileStream));
            }

            // Remove last message (its garbage)
            lastNMessagesFromGroup.pop_back();

            // Remove first messages
            while (lastNMessagesFromGroup.size() > m_nMessagesToRetrieve * m_nMessagesToRetrieve)
            {
                lastNMessagesFromGroup.pop_front();
            }
        }

        fileStream.close();
    }

    return lastNMessagesFromGroup;
}

std::pair<std::shared_ptr<CUser>, std::string>
CServer::login(int clientSocket)
{
    // Locks for login operation
    std::lock_guard lock{ g_loginMutex };

    // True value
    int optVal{ -1 };
    // Options size in bytes
    socklen_t cOptLen{ sizeof(int) };
    
    // Check the status for the keepalive option
    if (auto retVal{ getsockopt(clientSocket, SOL_SOCKET, SO_KEEPALIVE, &optVal, &cOptLen) };
        (optVal != 1) || (retVal < 0))
    {
        throw std::runtime_error("Client keep alive is not set");
    }

    // Connection closed?
    bool isConnectionClosed{ false };

    // Read login message
    const CMessage loginMessage{ CMessage::readMessageFromSocket(clientSocket, isConnectionClosed) };

    if (!isConnectionClosed)
    {
        std::runtime_error{ "Client closed connection during login" };
    }

    // Check if group is already instantiated
    if (auto groupIt{ m_groups->find(loginMessage.getGroupID()) };
        groupIt == m_groups->end())
    {
        // Add groupID to the map
        m_groups->insert({ loginMessage.getGroupID(), std::vector<int>{} });
    }

    // Register user into the group
    m_groups->at(loginMessage.getGroupID()).push_back(clientSocket);

    // Current user
    std::vector<std::shared_ptr<CUser>>::iterator currentUserIt{};

    // If user is already logged on, we add a new session
    if (auto it{ std::find_if(m_users.begin(), m_users.end(), [&](const auto& user) { return user->getUserID() == loginMessage.getUserID(); }) };
        it != m_users.end())
    {
        currentUserIt = it;
        // If cannot add to group, then the user already has the maximum number of sessions
        if (!(*currentUserIt)->addToGroup(loginMessage.getGroupID(), clientSocket))
        {
            // Remove user from group
            removeFromGroup(*currentUserIt, loginMessage.getGroupID());
            throw std::runtime_error("Maximum number of sessions for user user: " + loginMessage.getUserID());
        }
    }
    else
    {
        m_users.push_back(std::make_shared<CUser>(loginMessage.getUserID()));
        currentUserIt = std::prev(m_users.end());
        (*currentUserIt)->addToGroup(loginMessage.getGroupID(), clientSocket);
    }

    syncToDisk();

    // Get last messages from disk
    if (auto messageList{ retrieveLastNMessages(loginMessage.getGroupID()) }; !messageList.empty())
    {
        // Send all messages to the client
        for (const auto& message : messageList)
        {
            // Sleep between messages to avoid bombing the client socket
            usleep(100000);
            message.sendMessageToSocket(clientSocket);
        }
    }

    // Braoadcast user login
    const CMessage cBroadcastMessage{ "Server", loginMessage.getGroupID(),
        loginMessage.getUserID() + " has connected" };

    // Print info on server
    std::cout << "[INFO] " + loginMessage.getUserID() + " has connected" << std::endl;

    broadcastMessage(clientSocket, cBroadcastMessage, true);

    return { *currentUserIt, loginMessage.getGroupID() };
}

void
CServer::syncToDisk()
{
    // Blocks thread
    std::lock_guard<std::mutex> lock{ g_diskMutex };

    // Begin of the messages
    auto beginIt{ m_messageBuffer->begin() };
    // End of the messages
    auto endIt{ m_messageBuffer->end() };
    for (auto it{ beginIt }; it != endIt; ++it)
    {
        // Group file
        std::filesystem::path groupFilePath = m_groupFolderPath;
        groupFilePath.append((*it).getGroupID() + ".msg");
        // Open group file
        std::ofstream groupFile(groupFilePath, std::ios::out | std::ios::binary | std::ios::app);

        if (groupFile.good())
        {
            (*it).writeToDisk(groupFile);
        }

        groupFile.close();
    }
    // Remove all messages from buffer
    m_messageBuffer->clear();
}

void
CServer::removeFromGroup(std::shared_ptr<CUser> currentUser, const std::string& currentGroup)
{
    // Locks for removal
    std::lock_guard lock{ g_loginMutex };

    // Begin of group vector
    auto groupBeginIt{ m_groups->at(currentGroup).begin() };
    // End of group vector
    auto groupEndIt{ m_groups->at(currentGroup).end() };

    // Remove from group
    if (auto it{ std::find(groupBeginIt, groupEndIt, currentUser->removeSession(currentGroup)) };
        it != groupEndIt)
    {
        m_groups->at(currentGroup).erase(it);
    }
}

void 
CServer::broadcastMessage(int sendingClientSocket, const CMessage& message, bool sendToSendingClient)
{
    // Locks for broadcast operation
    std::lock_guard lock{ g_broadcastMutex };

    // Broadcast message
    for (const auto& user : m_groups->at(message.getGroupID()))
    {
        // Do not send message to who sent it
        if ((user != sendingClientSocket) || sendToSendingClient)
        {
            // Send message to client
            message.sendMessageToSocket(user);
        }
    }
}