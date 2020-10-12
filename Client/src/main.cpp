#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>
#include "CClient.h"

int main(int argc, char* argv[])
{
    CClient cClient{ argv[1], argv[2], argv[3], std::stoi(argv[4]) };

    return 0;
}