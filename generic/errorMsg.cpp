#include "generic/errorMsg.h"

namespace generic
{

void buildErrorString(const char *error, std::string *errorString, int indent)
{
    if(errorString==nullptr)
        return;

    for(int i=0; i<indent; ++i)
        errorString->append("\t");
    errorString->append(error);
    errorString->append("\n");
}

void buildErrorString(const std::string &error, std::string *errorString, int indent)
{
    buildErrorString(error.c_str(), errorString, indent);
}

void buildErrorString(const std::vector<std::string> &errors, std::string *errorString, int indent)
{
    for(const std::string &errorMsg:errors)
        buildErrorString(errorMsg, errorString, indent);
}

void buildErrorString(const char *error, const char *subError, std::string *errorString, int indent)
{
    buildErrorString(error, errorString, indent);
    buildErrorString(subError, errorString, indent+1);
}

void buildErrorString(const char *error, const std::string &subError, std::string *errorString, int indent)
{
    buildErrorString(error, errorString, indent);
    buildErrorString(subError, errorString, indent+1);
}

void buildErrorString(const char *error, const std::vector<std::string> &errors, std::string *errorString, int indent)
{
    buildErrorString(error, errorString, indent);
    buildErrorString(errors, errorString, indent+1);
}

void buildErrorString(const std::string &error, const std::vector<std::string> &errors, std::string *errorString, int indent)
{
    buildErrorString(error, errorString, indent);
    buildErrorString(errors, errorString, indent+1);
}

}//namespace generic