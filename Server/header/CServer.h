#ifndef CServer_h
#define CServer_h

#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdint>
#include <vector>
#include <thread>

#include "CGenericMonitor.h"

class CServer
{
public:
	// Initializes server class
	CServer(std::size_t numberOfMsgToRetrieve);

	// Class destructor
	~CServer();

	// Waits for new connections and create handler threads
	void waitForConnections();

private:
	// Number of past messages to be retrieved from file
	std::size_t m_nMessagesToRetrieve;

	// Server socket
	int m_serverSocket;

	// Server port
	std::uint16_t m_port;

	// Threads handling client connections (it needs to be a monitor to avoid data race condition)
	CGenericMonitor<std::vector<std::thread>> m_handlerThreads;

	// Handle connection with a client
	void handleClientConnection(int clientSocket);
};

#endif