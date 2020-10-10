#ifndef CGenericMonitor_h
#define CGenericMonitor_h

#include <mutex>

template<class T>
class CGenericMonitor
{
public:
    template<typename ...Args>
    CGenericMonitor(Args&&... args) : m_cl(std::forward<Args>(args)...) {}

    struct monitor_helper
    {
        monitor_helper(CGenericMonitor* mon) : m_mon(mon), m_ul(mon->m_lock) {}
        T* operator->() { return &m_mon->m_cl; }
        CGenericMonitor* m_mon;
        std::unique_lock<std::mutex> m_ul;
    };

    monitor_helper operator->() { return monitor_helper(this); }
    monitor_helper ManuallyLock() { return monitor_helper(this); }
    T& GetThreadUnsafeAccess() { return m_cl; }

private:
    T           m_cl;
    std::mutex  m_lock;
};

#endif // !CGenericMonitor_h