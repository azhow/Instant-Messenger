#ifndef CMessage_h
#define CMessage_h

#include <ctime>
#include <array>
#include <cstdint>

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