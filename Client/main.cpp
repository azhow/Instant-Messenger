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

#include "CMessage.h"

#define PORT 4000

int main(int argc, char* argv[])
{
    // Write dummy group file
    {
        std::ofstream wf("/home/lain/Groups/Patata.msg", std::ios::out | std::ios::binary);
        if (!wf) {
            std::cout << "Cannot open file!" << std::endl;
            return 1;
        }

        for (std::size_t idx{ 0 }; idx < 10; idx++)
        {

            // HOW TO SEND MESSAGE
            std::string messData{ "message number: " + std::to_string(idx) };
            CMessage message{ "Patati", "Patata", messData };
            CMessage::SMessage m{ message.serialize() };
            // Read header
            wf.write((char*) &m, sizeof(CMessage::SMessageHeader));
            // Read message data
            wf.write(m.m_data, m.m_header.m_messageSize);
        }

        wf.close();
        if (!wf.good()) {
            std::cout << "Error occurred at writing time!" << std::endl;
            return 1;
        }
    }



    int sockfd, n;
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

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        printf("ERROR connecting\n");

    CMessage loginMessage{ CMessage::loginMessage("Patati", "Patata") };
    CMessage::SMessage serialized{ loginMessage.serialize() };

    // Write message into the socket
    if (write(sockfd, &serialized, sizeof(CMessage::SMessage)) == -1)
    {
        // error
    }

    ///* read from the socket */
    //n = read(sockfd, buffer, 256);
    //if (n < 0)
    //    printf("ERROR reading from socket\n");

    //printf("%s\n", buffer);

    close(sockfd);
    return 0;
}