#ifndef CServer_h
#define CServer_h

#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdint>
#include <vector>
#include <thread>
#include <unordered_map>
#include <filesystem>

#include "CGenericMonitor.h"
#include "CMessage.h"

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

	// Message buffer
	CGenericMonitor<std::vector<CMessage>> m_messageBuffer;

	// Groups are ID -> vector of sockets
	CGenericMonitor<std::unordered_map<std::string, std::vector<int>>> m_groups;

	// Handle connection with a client
	void handleClientConnection(int clientSocket);

	// Save message buffer to disk
	void syncToDisk();

	// Register group (either reads from disk if already existent or creates a whole new group)
	// Returns the last N² messages if there's already a file for that group
	std::vector<CMessage> registerGroup(const std::string& groupID);

	// Read last N messages from group file on the disk
	std::vector<CMessage> retrieveLastNMessages(const std::filesystem::path& groupFilePath) const;
};

#endif