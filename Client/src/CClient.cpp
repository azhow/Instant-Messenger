#include "CClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>
#include <fstream>
#include <iostream>
#include <errno.h>
#include <future>
#include <unistd.h>
#include <exception>
#include <algorithm>
#include <fstream>
#include <thread>
#include <netinet/tcp.h>

CClient::CClient(std::string userName, std::string groupName, std::string server_ip_addr, std::uint16_t port) 
{
    m_port = port;
    m_userID = userName;
    m_groupID = groupName;

    struct sockaddr_in serv_addr;
    struct hostent* server;

    server = gethostbyname(server_ip_addr.c_str());
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    if ((m_clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        printf("ERROR opening socket\n");

    // Option value for option level
    const int cOptionValue{ 1 };

    // Keep alive idle time before first check and keep alive time in seconds
    const int cKeepAliveTime{ 60 };

    // Set keep alive
    if (setsockopt(m_clientSocket, SOL_SOCKET, SO_KEEPALIVE, &cOptionValue, sizeof(int)) == 0)
    {
        // Set keep timer
        if (setsockopt(m_clientSocket, SOL_TCP, TCP_KEEPIDLE, &cKeepAliveTime, sizeof(int)) == 0)
        {
            // Set keep alive
            if (setsockopt(m_clientSocket, SOL_TCP, TCP_KEEPINTVL, &cKeepAliveTime, sizeof(int)) != 0)
            {
                printf("ERROR seting KEEPINTVL\n");
            }
        }
        else
        {
            printf("ERROR seting KEEPIDLE\n");
        }
    }
    else
    {
        printf("ERROR seting KEEPALIVE\n");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(m_port);
    serv_addr.sin_addr = *((struct in_addr*)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    if (int retVal{ connect(m_clientSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) }; retVal < 0)
    {
        printf("ERROR connecting\n");
        printf("%d\n", errno);
    }

    // Login in the server
    CMessage::sendLoginMessage(m_clientSocket, m_userID, m_groupID);

    bool isConnectionClosed_w{ false };
    bool isConnectionClosed_r{ false };
    CMessage receivedMessage{ CMessage::readMessageFromSocket(m_clientSocket, isConnectionClosed_r) };
    if (!isConnectionClosed_r)
    {
        std::cout << receivedMessage.getPrintableMessage() << std::endl;
        std::cout << "========== END OF MESSAGE ==========" << std::endl;
    }

    // Create new handler thread for reading/writing
    std::thread t(&CClient::handleClientWriting, this, m_clientSocket, std::ref(isConnectionClosed_w));
    handleClientReading(m_clientSocket, isConnectionClosed_r);

    close(m_clientSocket);
}

void
CClient::handleClientReading(int m_clientSocket, bool& isConnectionClosed) {

    while (!isConnectionClosed)
    {
        std::shared_future<CMessage> ret = std::async(&CMessage::readMessageFromSocket, m_clientSocket, std::ref(isConnectionClosed));

        CMessage returnMessage = ret.get();
        if (!isConnectionClosed)
            std::cout << returnMessage.getPrintableMessage() << std::endl;
    }

}

void
CClient::handleClientWriting(int m_clientSocket, bool& isConnectionClosed) {
    while (!isConnectionClosed)
    {
        std::string messData;
        std::getline(std::cin, messData);
        CMessage message{ m_userID, m_groupID, messData };
        if (!std::cin.eof())
        {
            if (messData != "")
                isConnectionClosed = isConnectionClosed || (message.sendMessageToSocket(m_clientSocket) == 0);
        }
        else {
            exit(0);
        }
    }
}

CClient::~CClient() {
    //Close client socket
    close(m_clientSocket);
}