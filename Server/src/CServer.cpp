#include "CServer.h"

#include <unistd.h>
#include <strings.h>
#include <exception>
#include <algorithm>
#include <fstream>

CServer::CServer(std::size_t numberOfMsgToRetrieve, std::uint16_t port) :
    m_nMessagesToRetrieve(numberOfMsgToRetrieve),
    m_port(port)
{
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
    int optionValue{ 0 };
    
    // Set server socket to allow immediate reuse of the port
    if (setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEPORT, &optionValue, sizeof(int)) == 0)
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
        throw std::runtime_error("Could not set server socket options");
    }
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
    // Login logic
    // TODO need to be a separated method with more logic, such as the maximum number of sessions per user
    {
        bool isConnectionClosed{ false };

        // Read login message
        const CMessage loginMessage{ CMessage::readMessageFromSocket(clientSocket, isConnectionClosed) };

        // Check if group is already instantiated
        if (auto groupIt{ m_groups->find(loginMessage.getGroupID()) };
            groupIt == m_groups->end())
        {
            // Add groupID to the map
            m_groups->insert({ loginMessage.getGroupID(), std::vector<int>{} });
        }

        // Get last messages from disk
        if (auto messageList{ retrieveLastNMessages(loginMessage.getGroupID()) }; !messageList.empty())
        {
            // Send all messages to the client
            for (const auto& message : messageList)
            {
                message.sendMessageToSocket(clientSocket);
            }
        }

        // Register user into the group
        m_groups->at(loginMessage.getGroupID()).push_back(clientSocket);

        // Braoadcast user login
        const CMessage cBroadcastMessage{ loginMessage.getUserID(), loginMessage.getGroupID(),
            loginMessage.getUserID() + " has connected" };

        // Broadcast message
        for (const auto& user : m_groups->at(cBroadcastMessage.getGroupID()))
        {
            // Send message to client
            cBroadcastMessage.sendMessageToSocket(user);
        }
    }

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
            // Broadcast message
            for (const auto& user : m_groups->at(cReceivedMessage.getGroupID()))
            {
                // Send message to client
                cReceivedMessage.sendMessageToSocket(user);
            }
        }
    }

    // Remove client from group
    // TODO

    // Close client socket
    close(clientSocket);
}

std::list<CMessage>
CServer::retrieveLastNMessages(const std::string& groupID) const
{
    // Last messages from group
    std::list<CMessage> lastNMessagesFromGroup{};

    // Path to the group's file
    std::filesystem::path groupFilePath{ std::filesystem::current_path() };
    // Groups folder
    groupFilePath.append("Groups");
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