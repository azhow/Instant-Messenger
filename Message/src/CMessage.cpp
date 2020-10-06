#include "CMessage.h"

#include <chrono>
#include <string.h>

CMessage::CMessage(const std::string& userID, const std::string& groupID, const std::string& messageData) :
    m_timestamp(std::chrono::seconds(std::time(NULL)).count()),
    m_userID(userID),
    m_groupID(groupID),
    m_messageData(messageData)
{

}

CMessage::CMessage(std::time_t timestamp, const std::string& userID, const std::string& groupID, const std::string& messageData) :
    m_timestamp(timestamp),
    m_userID(userID),
    m_groupID(groupID),
    m_messageData(messageData)
{

}

CMessage::~CMessage()
{

}

CMessage::SMessage
CMessage::serialize()
{
    // Serialized message header
    SMessageHeader serializedMessageHeader;

    // Serialized message
    SMessage serializedMessage;

    serializedMessageHeader.m_timestamp = m_timestamp;
    // Copy user ID to header struct
    strncpy(serializedMessageHeader.m_userID, m_userID.c_str(), m_cIDSize);
    // Copy group ID to header
    strncpy(serializedMessageHeader.m_groupID, m_groupID.c_str(), m_cIDSize);
    serializedMessageHeader.m_messageSize = m_messageData.size();
    serializedMessage.m_header = serializedMessageHeader;
    // Allocate memory for the message data
    serializedMessage.m_data = new char[m_messageData.size()];
    // Copy message to struct
    strncpy(serializedMessage.m_data, m_messageData.c_str(), m_messageData.size());

    return serializedMessage;
}

CMessage
CMessage::deserialize(const CMessage::SMessage& message)
{
    return CMessage(message.m_header.m_timestamp, message.m_header.m_userID, 
        message.m_header.m_groupID, message.m_data);
}