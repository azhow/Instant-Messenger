#include "CMessage.h"

#include <chrono>

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
    return SMessage(m_timestamp, m_userID, m_groupID, m_messageData);
}

CMessage
CMessage::deserialize(const CMessage::SMessage& message)
{
    return CMessage(message.m_header.m_timestamp, message.m_header.m_userID, 
        message.m_header.m_groupID, message.m_data);
}

CMessage
CMessage::loginMessage(std::string_view userID, std::string_view groupID)
{
    return CMessage(std::string(userID), std::string(groupID), "");
}