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
#include <sys/ioctl.h>
#include <errno.h>

#include "CMessage.h"

#define PORT 4020

int main(int argc, char* argv[])
{
    //// Write dummy group file
    //{
    //    std::ofstream wf("/home/adpeiter/Groups/Patata.msg", std::ios::out | std::ios::binary);
    //    if (!wf) {
    //        std::cout << "Cannot open file!" << std::endl;
    //        return 1;
    //    }

    //    for (std::size_t idx{ 0 }; idx < 1000; idx++)
    //    {
    //        std::string messData{ "message number: " + std::to_string(idx) };
    //        CMessage message{ "Patati", "Patata", messData };
    //        message.writeToDisk(wf);
    //    }

    //    wf.close();
    //    if (!wf.good()) {
    //        std::cout << "Error occurred at writing time!" << std::endl;
    //        return 1;
    //    }
    //}


    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent* server;


    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        printf("ERROR opening socket\n");


    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr = *((struct in_addr*)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    if (int retVal{ connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) }; retVal < 0)
    {
        printf("ERROR connecting\n");
        printf("%d\n", errno);
    }


    // Login in the server
    CMessage::sendLoginMessage(sockfd, argv[1], argv[2]);

    bool isConnectionClosed{ false };
    while (!isConnectionClosed)
    {
        CMessage receivedMessage{ CMessage::readMessageFromSocket(sockfd, isConnectionClosed) };
        if (!isConnectionClosed)
        {
            std::cout << receivedMessage.getMessageData() << std::endl;
            std::cout << "========== END OF MESSAGE ==========" << std::endl;
        }
    }
    ///* read from the socket */
    //n = read(sockfd, buffer, 256);
    //if (n < 0)
    //    printf("ERROR reading from socket\n");

    //printf("%s\n", buffer);

    close(sockfd);
    return 0;
}