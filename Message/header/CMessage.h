#ifndef CMessage_h
#define CMessage_h

#include <ctime>
#include <array>
#include <cstdint>
#include <string.h>
#include <fstream>
#include <future>

class CMessage
{
public:
    // Message constructor which generates timestamp
    CMessage(const std::string& userID, const std::string& groupID, const std::string& messageData);

    // Message constructor that already has a timestamp
    CMessage(std::time_t timestamp, const std::string& userID, const std::string& groupID, const std::string& messageData);

    // Message destructor
    ~CMessage();

    // Creates login message for user into a group
    static bool sendLoginMessage(int serverSocketFD, const std::string& userID, const std::string& groupID);

    // Send message to socket
    bool sendMessageToSocket(int socketFD, std::size_t sendBufferSize) const;

    // Read message from socket
    static CMessage readMessageFromSocket(int socketFD, bool& isConnectionClosed);

    // Write message to disk on a opened existing file
    bool writeToDisk(std::ofstream& outputFile) const;

    // Read message from disk
    static CMessage readMessageFromDisk(std::ifstream& inputFile);

    // Is the message body empty?
    inline bool isMessageEmpty() const { return m_messageData.empty(); };

    // Get userID
    inline std::string getUserID() const { return m_userID; };

    // Get groupID
    inline std::string getGroupID() const { return m_groupID; };

    //Get messData
    inline std::string getmessData() const { return m_messageData; };

    // Transform message into a printable pretty using timestamp, userID and messageData as a string
    std::string getPrintableMessage() const;

private:
    // UNIX timestamp of the message
    std::time_t m_timestamp;

    // User ID that sent the message
    std::string m_userID;

    // Group ID to which the message was sent
    std::string m_groupID;

    // Message data
    std::string m_messageData;

    // ID size for groups and users
    static inline const std::size_t m_cIDSize{ 20 };

    // Message header struct
    struct SMessageHeader
    {
        std::time_t m_timestamp;
        std::uint64_t m_messageSize;
        char m_userID[m_cIDSize] = { 0 };
        char m_groupID[m_cIDSize] = { 0 };

        SMessageHeader() :
            m_timestamp{ 0 },
            m_messageSize{ 0 }
        {

        }

        SMessageHeader(std::time_t timestamp,
            const std::string& userID,
            const std::string& groupID,
            std::uint64_t messageDataSize)
        {
            m_timestamp = timestamp;
            // Copy user ID to header struct
            strncpy(m_userID, userID.c_str(), m_cIDSize);
            // Copy group ID to header
            strncpy(m_groupID, groupID.c_str(), m_cIDSize);
            m_messageSize = messageDataSize;
        }
    };

    // Message struct to be serialized
    struct SMessage
    {
        SMessageHeader m_header;
        char* m_data;

        // Struct constructor
        SMessage(std::time_t timestamp, const std::string& userID,
            const std::string& groupID, const std::string& messageData) :
            m_header{ SMessageHeader{timestamp, userID, groupID, messageData.size() + 1} }
        {
            // Allocate memory for the message data
            m_data = new char[m_header.m_messageSize];
            // Copy message to struct
            strncpy(m_data, messageData.c_str(), messageData.size());
            m_data[messageData.size()] = '\0';
        }

        // Struct constructor
        SMessage(SMessageHeader& header) :
            m_header(header),
            m_data(nullptr)
        {
            if (m_header.m_messageSize >= 1)
            {
                // Allocate memory for the message data
                m_data = new char[m_header.m_messageSize];
            }
        }

        // Struct destructor (deallocates string data)
        ~SMessage() { if (m_data != nullptr) delete[] m_data; }
    };

    // Serialize
    SMessage serialize() const;

    // Deserialize
    static CMessage deserialize(const SMessage& message);
};

#endif // !CMessage_h