#include "generic/log.h"

namespace generic
{

std::vector<SharedLogCallback> Log::m_callbacks;
std::string Log::emptyString;

const std::string &logLevelString(LogLevel level)
{
    static std::vector<std::string> logLevelNames={"Message", "Warning", "Error", "Debug"};

    return logLevelNames[(size_t)level];
}

void Log::attachCallback(SharedLogCallback callback)
{ m_callbacks.push_back(callback); }

void Log::attachCallbackTop(SharedLogCallback callback)
{
    m_callbacks.insert(m_callbacks.begin(), callback);
}

void Log::dettachCallback(SharedLogCallback callback)
{
    std::vector<SharedLogCallback>::iterator iter=std::find(m_callbacks.begin(), m_callbacks.end(), callback);

    if(iter!=m_callbacks.end())
        m_callbacks.erase(iter);
}

void Log::write(LogLevel level, const std::string &entry)
{
    for(SharedLogCallback &callback:m_callbacks)
    {
        if(callback->write(level, entry))
            break;
    }
}

void Log::message(const std::string &entry)
{
    write(LogLevel::Message, entry);
}

void Log::warning(const std::string &entry)
{
    write(LogLevel::Warning, entry);
}

void Log::error(const std::string &entry)
{
    write(LogLevel::Error, entry);
}

void Log::debug(const std::string &entry)
{
    write(LogLevel::Debug, entry);
}


//void Log::write(QString entry)
//{
//	BOOST_FOREACH(SharedLogCallback &callback, m_callbacks)
//	{
//		callback->write(entry.toStdString());
//	}
//}

}//namespace generic