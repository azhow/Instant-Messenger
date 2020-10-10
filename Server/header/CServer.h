#ifndef CServer_h
#define CServer_h

#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdint>
#include <vector>
#include <list>
#include <thread>
#include <unordered_map>
#include <filesystem>

#include "CGenericMonitor.h"
#include "CMessage.h"
#include "CUser.h"

class CServer
{
public:
	// Initializes server class
	CServer(std::size_t numberOfMsgToRetrieve, std::uint16_t port);

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

	// Group folder path
	std::filesystem::path m_groupFolderPath;

	// Threads handling client connections (it needs to be a monitor to avoid data race condition)
	CGenericMonitor<std::vector<std::thread>> m_handlerThreads;

	// Message buffer
	CGenericMonitor<std::vector<CMessage>> m_messageBuffer;

	// Groups are ID -> vector of sockets
	CGenericMonitor<std::unordered_map<std::string, std::vector<int>>> m_groups;

	// Vector of users
	std::vector <std::shared_ptr<CUser>> m_users;

	// Handle connection with a client
	void handleClientConnection(int clientSocket);

	// Save message buffer to disk
	void syncToDisk();

	// Returns the last N² messages if there's already a file for that group
	std::list<CMessage> retrieveLastNMessages(const std::string& groupID) const;

	// Login a user into the server
	std::pair<std::shared_ptr<CUser>, std::string> login(int clientSocket);

	// Removes user from a group
	void removeFromGroup(std::shared_ptr<CUser> currentUser, const std::string& currentGroup);
};

#endif