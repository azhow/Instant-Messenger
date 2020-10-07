#include "CServer.h"

#include <unistd.h>
#include <strings.h>
#include <exception>

CServer::CServer(std::size_t numberOfMsgToRetrieve) :
    m_nMessagesToRetrieve(numberOfMsgToRetrieve),
    m_port(4000)
{
    // Socket file descriptor
    m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (m_serverSocket == -1)
    {
        // Error
        throw "Could not create server socket";
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
    if (setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &optionValue, sizeof(int)) == 0)
    {
        // Bind socket to address
        if (bind(m_serverSocket, (struct sockaddr*)&servAddr, sizeof(servAddr)) != 0)
        {
            // Error
            throw "Could not bind server socket";
        }
    }
    else
    {
        // Error
        throw "Could not set server socket options";
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
            throw "Could not listen for new connections";
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
            throw "Could not accept incoming connection";
        }

        // Create new handler thread for connection
        m_handlerThreads->push_back(std::thread(&CServer::handleClientConnection, this, clientSocket));
    }
}

void
CServer::handleClientConnection(int clientSocket)
{
    // Message header from received message
    CMessage::SMessageHeader messageHeader;

    // Read message header
    if (read(clientSocket, &messageHeader, sizeof(messageHeader)) == -1)
    {
        // Error
    }

    // Check if group is already instantiated
    if (auto groupIt{ m_groups->find(messageHeader.m_groupID) };
        groupIt == m_groups->end())
    {
        // Register group
        registerGroup(messageHeader.m_groupID);
    }

    // Register user into the group
    m_groups->at(messageHeader.m_groupID).push_back(clientSocket);

    // TODO when does the client connection closes?
    // Maybe we need to create a close message or timeout for the socket
    while (true)
    {
        // TODO handle incoming messages
    }

    // Close client socket
    close(clientSocket);
}