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

CClient::CClient(std::string userName, std::string groupName, std::string server_ip_addr, std::uint16_t port) {


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

    bool isConnectionClosed{ false };
    while (!isConnectionClosed)
    {
        CMessage receivedMessage{ CMessage::readMessageFromSocket(m_clientSocket, isConnectionClosed) };
        if (!isConnectionClosed)
        {
            std::cout << receivedMessage.getMessageData() << std::endl;
            std::cout << "========== END OF MESSAGE ==========" << std::endl;
        }


        // Create new handler thread for reading/writing
        /*m_readingThreads->push_back(*/handleClientReading(m_clientSocket, isConnectionClosed);
        /*m_writingThreads->push_back(*/std::thread(&CClient::handleClientWriting, this, m_clientSocket);



    }
    ///* read from the socket */
    //n = read(sockfd, buffer, 256);
    //if (n < 0)
    //    printf("ERROR reading from socket\n");

    //printf("%s\n", buffer);

    close(m_clientSocket);
}

void
CClient::handleClientReading(int m_clientSocket, bool& isConnectionClosed) {



    std::shared_future<CMessage> ret = std::async(&CMessage::readMessageFromSocket, m_clientSocket, std::ref(isConnectionClosed));

    CMessage returnMessage = ret.get();
    if (!isConnectionClosed)
    {
        std::cout << returnMessage.getMessageData() << std::endl;
    }


}

void
CClient::handleClientWriting(int m_clientSocket) {

    std::string messData;
    std::getline(std::cin, messData);
    CMessage message{ m_userID, m_groupID, messData };
    message.sendMessageToSocket(m_clientSocket);

}

CClient::~CClient() {
    //Close client socket
    close(m_clientSocket);
}