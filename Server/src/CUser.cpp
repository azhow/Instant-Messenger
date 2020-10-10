#include "CUser.h"

// User mutex
std::mutex g_userLock;

bool 
CUser::addToGroup(const std::string& groupID, int socketFD)
{
    // Locks for operation
    std::lock_guard{ g_userLock };

    // If could add to group
    bool retVal{ false };

    if (m_sessions.size() < 3)
    {
        m_sessions.push_back({ groupID, socketFD });
        retVal = true;
    }

    return retVal;
}

int
CUser::removeSession(const std::string& groupID)
{
    // Locks for operation
    std::lock_guard{ g_userLock };

    // Return value
    int retVal;

    auto beginIt{ m_sessions.begin() };
    auto endIt{ m_sessions.end() };

    // Remove if the user is in the group
    if (auto session{ std::find_if(beginIt, endIt, [&](const auto& session) { return session.first == groupID; }) };
        session != endIt)
    {
        retVal = (*session).second;
        m_sessions.erase(session);
    }

    return retVal;
}