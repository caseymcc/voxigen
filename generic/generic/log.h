#ifndef _generic_log_h_
#define _generic_log_h_

#include "generic/generic_export.h"

#include <vector>
#include <memory>

#pragma warning(push)
#pragma warning(disable:4251)

namespace generic
{

enum class LogLevel
{
	Message=0,
	Warning=1,
	Error=2,
	Debug=3
};

GENERIC_EXPORT const std::string &logLevelString(LogLevel);

class GENERIC_EXPORT ILogCallback
{
public:
	ILogCallback(){};
	virtual ~ILogCallback(){};

	virtual bool write(LogLevel level, const std::string &entry)=0;
};
typedef std::shared_ptr<ILogCallback> SharedLogCallback;

class GENERIC_EXPORT Log
{
public:
	Log(){};
	~Log(){};

	static void attachCallback(SharedLogCallback callback);
	static void attachCallbackTop(SharedLogCallback callback);
	static void dettachCallback(SharedLogCallback callback);

	static void write(LogLevel level, const std::string &entry);

	static void message(const std::string &entry);
	static void warning(const std::string &entry);
	static void error(const std::string &entry);
	static void debug(const std::string &entry);

    template<typename ..._ArgTypes>
    static void write(LogLevel level, const std::string &format, _ArgTypes... args);

    template<typename ..._ArgTypes>
    static void message(const std::string &format, _ArgTypes... args);
    template<typename ..._ArgTypes>
    static void warning(const std::string &format, _ArgTypes... args);
    template<typename ..._ArgTypes>
    static void error(const std::string &format, _ArgTypes... args);
    template<typename ..._ArgTypes>
    static void debug(const std::string &format, _ArgTypes... args);

private:
	static std::vector<SharedLogCallback> m_callbacks;
	static std::string emptyString;
};

template<typename ..._ArgTypes>
void Log::write(LogLevel level, const std::string &format, _ArgTypes... args)
{
    size_t size=std::snprintf(nullptr, 0, format.c_str(), args...);

    std::string entry(size, 0);

    std::snprintf(&entry[0], size+1, format.c_str(), args...);
    Log::write(level, entry);
}

template<typename ..._ArgTypes>
void Log::message(const std::string &format, _ArgTypes... args)
{
    Log::write(LogLevel::Message, format, args...);
}

template<typename ..._ArgTypes>
void Log::warning(const std::string &format, _ArgTypes... args)
{
    Log::write(LogLevel::Warning, format, args...);
}

template<typename ..._ArgTypes>
void Log::error(const std::string &format, _ArgTypes... args)
{
    Log::write(LogLevel::Error, format, args...);
}

template<typename ..._ArgTypes>
void Log::debug(const std::string &format, _ArgTypes... args)
{
    Log::write(LogLevel::Debug, format, args...);
}

}//namespace voxigen

#pragma warning(pop)

#endif //_voxigen_log_h_