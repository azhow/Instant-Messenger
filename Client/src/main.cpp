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
#include <future>
#include "CMessage.h"
#include "CClient.h"
#include <thread>

int main(int argc, char* argv[])
{
    //// Write dummy group file
    //{
    //    std::ofstream wf("/home/lain/Groups/chaves.msg", std::ios::out | std::ios::binary);
    //    if (!wf) {
    //        std::cout << "Cannot open file!" << std::endl;
    //        return 1;
    //    }

    //    for (std::size_t idx{ 0 }; idx < 1000; idx++)
    //    {
    //        std::string messData{ "message number: " + std::to_string(idx) };
    //        CMessage message{ "gordo", "chaves", messData };
    //        message.writeToDisk(wf);
    //    }

    //    wf.close();
    //    if (!wf.good()) {
    //        std::cout << "Error occurred at writing time!" << std::endl;
    //        return 1;
    //    }
    //}

    CClient cClient{ argv[1], argv[2], argv[3], std::stoi(argv[4]) };




    return 0;
}