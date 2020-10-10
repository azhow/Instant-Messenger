#ifndef CUser_h
#define CUser_h

#include <vector>
#include <string>
#include <algorithm>
#include <utility>
#include <mutex>

class CUser
{
public:
    // User constructor
    CUser() : 
        m_userID{ "" },
        m_sessions{ }
    {};

    // User constructor
    CUser(const std::string& userID) :
        m_userID{ userID },
        m_sessions{ }
    {

    };

    // Add group reference to user
    bool addToGroup(const std::string& groupID, int socketFD);

    // Remove from group
    // Return iterator of position in vector of users
    int removeSession(const std::string& groupID);

    // Returns the user ID
    inline std::string getUserID() const { return m_userID; };

private:
    // User identification
    std::string m_userID;

    // Groups references
    std::vector<std::pair<std::string, int>> m_sessions;
};

#endif // !CUser_h