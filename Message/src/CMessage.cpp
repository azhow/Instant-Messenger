#include "CMessage.h"

#include <unistd.h>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>

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
    // Message data
    std::string messageData{ "" };

    // Check content
    if (message.m_data != nullptr)
    {
        messageData = std::string{ message.m_data };
    }

    return CMessage(message.m_header.m_timestamp, message.m_header.m_userID,
        message.m_header.m_groupID, messageData);
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
CMessage::sendMessageToSocket(int socketFD, std::size_t sendBufferSize) const
{
    // Return value
    bool retVal{ false };

    // Receive buffer size
    std::size_t receiveBufferSize;
    // Size of size_t type
    unsigned int sizeOfSizeT{ sizeof(std::size_t) };
    // Read buffer size from socket
    if (getsockopt(socketFD, SOL_SOCKET, SO_RCVBUF, (void*)&receiveBufferSize, &sizeOfSizeT) == 0)
    {
        // This message serialized
        SMessage serializedMessage{ serialize() };
        // Split message if bigger than this
        std::size_t splitSize{ std::min(receiveBufferSize, sendBufferSize) };

        // Message header is always smaller than the buffer size
        retVal = write(socketFD, &serializedMessage.m_header, sizeof(SMessageHeader)) > 0;

        if (serializedMessage.m_header.m_messageSize > splitSize)
        {
            std::size_t sentBytes{ 0 };
            std::size_t splitNum{ 0 };
            std::size_t bytesToSend{ serializedMessage.m_header.m_messageSize };
            while (sentBytes < serializedMessage.m_header.m_messageSize)
            {
                // Number of bytes that will be sent on this split
                std::size_t numBytesToSendThisIteration{ 0 };
                (bytesToSend < splitSize) ? numBytesToSendThisIteration = bytesToSend : numBytesToSendThisIteration = splitSize;
                sentBytes += write(socketFD, serializedMessage.m_data, numBytesToSendThisIteration);
                bytesToSend -= numBytesToSendThisIteration;
                // Couldn`t send bytes
                if (sentBytes < 0)
                {
                    retVal = false;
                    break;
                }
            }
        }
        else
        {
            // Message smaller than buffers then its ok
            retVal = retVal && (write(socketFD, serializedMessage.m_data, serializedMessage.m_header.m_messageSize) > 0);
        }
    }

    return retVal;
}

CMessage
CMessage::readMessageFromSocket(int socketFD, bool& isConnectionClosed)
{
    // Message header from received message
    SMessageHeader messageHeader;

    // Number of bytes read
    auto bytesRead = read(socketFD, &messageHeader, sizeof(SMessageHeader));
    isConnectionClosed = bytesRead == 0;

    // Read message header
    while ((bytesRead != sizeof(SMessageHeader)) && (bytesRead > 0))
    {
        bytesRead += read(socketFD, &messageHeader, sizeof(SMessageHeader) - bytesRead);
        isConnectionClosed = bytesRead == 0;
    }

    // Parse message data
    SMessage readMessage{ messageHeader };

    // If theres a message body to read we read it
    if (readMessage.m_header.m_messageSize > 1)
    {
        bytesRead = read(socketFD, readMessage.m_data, messageHeader.m_messageSize);
        
        while ((bytesRead != messageHeader.m_messageSize) && (bytesRead > 0))
        {
            bytesRead += read(socketFD, &messageHeader, messageHeader.m_messageSize - bytesRead);
            isConnectionClosed = bytesRead == 0;
        }
    }

    return deserialize(readMessage);
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