#ifndef _voxigen_loadprogress_h_
#define _voxigen_loadprogress_h_

#include <mutex>
#include <string>

class LoadProgress
{
public:
    LoadProgress(){}

    void update(std::string status, int progress, bool complete)
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        m_status=status;
        m_progress=progress;
        m_complete=complete;
    }

    void get(std::string &status, int &progress, bool &complete)
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        status=m_status;
        progress=m_progress;
        complete=m_complete;
    }

private:
    std::mutex m_mutex;
    std::string m_status;
    int m_progress;
    bool m_complete;
};

#endif //_voxigen_loadprogress_h_