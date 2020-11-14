#ifndef CClient_h
#define CClient_h

#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdint>
#include <vector>
#include <list>
#include <thread>
#include <unordered_map>
#include <filesystem>

#include "CMessage.h"
#include "CGenericMonitor.h"

class CClient
{
public:
	// Initializes client class
	CClient(std::string userName, std::string groupName, std::string server_ip_addr, std::uint16_t port);

	// Class destructor
	~CClient();

	// Waits for new connections and create handler threads
	void waitForConnections();

private:

	// Client socket
	int m_clientSocket;

	std::string m_userID;

	std::string m_groupID;

	// Client port
	std::uint16_t m_port;

	// Send buffer size
	std::size_t m_sendBufferSize;

	// Threads handling client read/write (it needs to be a monitor to avoid data race condition)
	CGenericMonitor<std::vector<std::thread>> m_readingThreads;
	CGenericMonitor<std::vector<std::thread>> m_writingThreads;

	// Message buffer
	CGenericMonitor<std::vector<CMessage>> m_messageBuffer;

	// Groups are ID -> vector of sockets
	CGenericMonitor<std::unordered_map<std::string, std::vector<int>>> m_groups;

	// Handle client reading messages from server
	void handleClientReading(int clientSocket, bool& isConnectionClosed);

	// Handle client writing messages to server
	void handleClientWriting(int clientSocket, bool& isConnectionClosed);

	// Save message buffer to disk
	void syncToDisk();

	// Returns the last N² messages if there's already a file for that group
	std::list<CMessage> retrieveLastNMessages(const std::string& groupID) const;
};

#endif
