#include "CServer.h"
#include "CMessage.h"

#include <iostream>

int main(int argc, char *argv[])
{
	try 
	{
		// Server object
		CServer cServer{ 10 };

		// Wait for connections and handles them
		cServer.waitForConnections();
	}
	catch (std::exception& e)
	{
		// Print exception message
		std::cout << e.what() << std::endl;
	}

	//CMessage mess{ "patati", "patatá", "oi pateta!" };

	//CMessage::SMessage serial{ mess.serialize() };

	//CMessage deserial{ CMessage::deserialize(serial) };

	return 0;
}