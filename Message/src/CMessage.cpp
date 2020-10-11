#include "CMessage.h"

#include <unistd.h>
#include <chrono>
#include <iomanip>

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
CMessage::serialize() const
{
    return SMessage(m_timestamp, m_userID, m_groupID, m_messageData);
}

CMessage
CMessage::deserialize(const CMessage::SMessage& message)
{
    return CMessage(message.m_header.m_timestamp, message.m_header.m_userID,
        message.m_header.m_groupID, message.m_data);
}

bool
CMessage::sendLoginMessage(int serverSocketFD, const std::string& userID, const std::string& groupID)
{
    // Return value
    bool retVal{ false };

    // Login message
    CMessage loginMessage{ CMessage(userID, groupID, "") };
    CMessage::SMessage serialized{ loginMessage.serialize() };

    // Write message into the server socket
    retVal = write(serverSocketFD, &serialized.m_header, sizeof(SMessageHeader)) > 0;

    return retVal;
}

bool
CMessage::sendMessageToSocket(int socketFD) const
{
    // Return value
    bool retVal{ false };

    // This message serialized
    SMessage serializedMessage{ serialize() };

    // Send message to client
    retVal = write(socketFD, &serializedMessage.m_header, sizeof(SMessageHeader)) > 0;
    retVal = retVal && (write(socketFD, serializedMessage.m_data, serializedMessage.m_header.m_messageSize) > 0);

    return retVal;
}

CMessage
CMessage::readMessageFromSocket(int socketFD, bool& isConnectionClosed)
{
    // Message header from received message
    SMessageHeader messageHeader;

    // Read message header
    isConnectionClosed = read(socketFD, &messageHeader, sizeof(SMessageHeader)) == 0;

    // Parse message data
    SMessage message{ messageHeader };

    // If theres a message body to read we read it
    if (message.m_header.m_messageSize > 1)
    {
        isConnectionClosed = isConnectionClosed || (
            read(socketFD, message.m_data, messageHeader.m_messageSize) == 0);
    }

    return deserialize(message);
}

bool
CMessage::writeToDisk(std::ofstream& outputFile) const
{
    // Return value
    bool retVal{ false };

    // Serializes message
    CMessage::SMessage serializedMessage{ serialize() };
    // Read header
    outputFile.write((char*)&serializedMessage, sizeof(SMessageHeader));
    retVal = outputFile.good();
    // Read message data
    outputFile.write(serializedMessage.m_data, serializedMessage.m_header.m_messageSize);
    retVal = retVal & outputFile.good();

    return retVal;
}

CMessage
CMessage::readMessageFromDisk(std::ifstream& inputFile)
{
    // First reads the message header to get the message size
    SMessageHeader header;
    inputFile.read((char*)&header, sizeof(SMessageHeader));
    // Then create the message and read the message data
    SMessage message{ header };
    inputFile.read(message.m_data, header.m_messageSize);
    
    // Deserialize message
    return deserialize(message);
}

std::string 
CMessage::getPrintableMessage() const
{
    // Printable string
    std::string retVal{ "[" };

    std::tm* t = localtime(&m_timestamp);
    std::stringstream ss;
    ss << std::put_time(t, "%Y-%m-%d %I:%M:%S %p");
    retVal += ss.str();

    retVal += "] " + m_userID + ": " + m_messageData;

    return retVal;
}