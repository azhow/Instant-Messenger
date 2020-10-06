#include "CServer.h"

int main(int argc, char *argv[])
{
	// Server object
	CServer cServer{ 10 };

	// Wait for connections and handles them
	cServer.waitForConnections();

	return 0;
}
