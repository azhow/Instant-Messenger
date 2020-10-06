#ifndef CMessage_h
#define CMessage_h

#include <ctime>
#include <array>
#include <cstdint>
#include <string.h>

class CMessage
{
public:
    // ID size for groups and users
    static inline const std::size_t m_cIDSize{ 20 };

    // Message header struct
    struct SMessageHeader
    {
        std::time_t m_timestamp;
        char m_userID[m_cIDSize];
        char m_groupID[m_cIDSize];
        std::uint64_t m_messageSize;
    };

    // Message struct to be serialized
    struct SMessage
    {
        SMessageHeader m_header;
        char* m_data;

        // Struct constructor
        SMessage(std::time_t timestamp, const std::string& userID, 
            const std::string& groupID, const std::string& messageData)
        {
            m_header.m_timestamp = timestamp;
            // Copy user ID to header struct
            strncpy(m_header.m_userID, userID.c_str(), m_cIDSize);
            // Copy group ID to header
            strncpy(m_header.m_groupID, groupID.c_str(), m_cIDSize);
            m_header.m_messageSize = messageData.size();
            // Allocate memory for the message data
            m_data = new char[messageData.size()];
            // Copy message to struct
            strncpy(m_data, messageData.c_str(), messageData.size());
        }

        // Struct destructor (deallocates string data)
        ~SMessage() { delete[] m_data; }
    };
    
    // Message constructor which generates timestamp
    CMessage(const std::string& userID, const std::string& groupID, const std::string& messageData);

    // Message constructor that already has a timestamp
    CMessage(std::time_t timestamp, const std::string& userID, const std::string& groupID, const std::string& messageData);

    // Message destructor
    ~CMessage();

    // Serialize
    SMessage serialize();

    // Deserialize
    static CMessage deserialize(const SMessage& message);

private:
    // UNIX timestamp of the message
    std::time_t m_timestamp;

    // User ID that sent the message
    std::string m_userID;

    // Group ID to which the message was sent
    std::string m_groupID;

    // Message data
    std::string m_messageData;
};

#endif // !CMessage_h