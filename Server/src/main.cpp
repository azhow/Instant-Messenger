#include "CServer.h"
#include "CMessage.h"

#include <iostream>

int main(int argc, char *argv[])
{
	try
	{
		// Server object
		CServer cServer{ 5, 6969 };

		// Wait for connections and handles them
		cServer.waitForConnections();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	return 0;
}