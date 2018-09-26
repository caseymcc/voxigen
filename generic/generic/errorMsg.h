#ifndef _generic_errorMsg_h_
#define _generic_errorMsg_h_

#include <string>
#include <vector>

namespace generic
{

template<typename ..._ArgTypes>
void addError(std::string *errorString, std::string format, _ArgTypes... args)
{
    if(!errorString)
        return;

    size_t pos=errorString->size();
    size_t size=std::snprintf(nullptr, 0, format.c_str(), args...)+1;
    
    errorString->resize(pos+size);
    std::snprintf(&errorString[pos], size, format.c_str(), args...);
}

void buildErrorString(const char *error, std::string *errorString, int indent=0);
void buildErrorString(const std::string &error, std::string *errorString, int indent=0);
void buildErrorString(const std::vector<std::string> &errors, std::string *errorString, int indent=0);

void buildErrorString(const char *error, const char *subError, std::string *errorString, int indent=0);
void buildErrorString(const char *error, const std::string &subError, std::string *errorString, int indent=0);

void buildErrorString(const char *error, const std::vector<std::string> &errors, std::string *errorString, int indent=0);
void buildErrorString(const std::string &error, const std::vector<std::string> &errors, std::string *errorString, int indent=0);

}//namespace generic

#endif//_generic_errorMsg_h_