#ifndef _generic_Serializer_h_
#define _generic_Serializer_h_

#include "generic/generic_export.h"
#include "generic/types.h"

#include <string>

namespace generic
{

class GENERIC_EXPORT Serializer
{
public:
	Serializer() {}
	virtual ~Serializer() {}

	virtual void startObject()=0;
	virtual void endObject()=0;

	virtual void startArray()=0;
	virtual void endArray()=0;

	virtual void addKey(const std::string key)=0;
	virtual void addBool(const bool &value)=0;
	virtual void addString(const std::string &value)=0;
	virtual void addInt(const int &value)=0;
	virtual void addUInt(const unsigned int &value)=0;
	virtual void addInt64(const int64_t &value)=0;
	virtual void addUInt64(const uint64_t &value)=0;
	virtual void addFloat(const float &value)=0;
	virtual void addDouble(const double &value)=0;
};

class GENERIC_EXPORT Deserializer
{
public:
    Deserializer() {}
	virtual ~Deserializer() {}

	virtual bool key(const std::string &key)=0;
	virtual Type type()=0;

	virtual std::string name()=0;

	virtual bool openObject()=0;
	virtual void closeObject()=0;

	virtual bool openArray()=0;
	virtual void closeArray()=0;

    virtual bool advance()=0;

	virtual bool getBool()=0;
	virtual std::string getString()=0;
	virtual int getInt()=0;
	virtual unsigned int getUInt()=0;
	virtual int64_t getInt64()=0;
	virtual uint64_t getUInt64()=0;
	virtual float getFloat()=0;
	virtual double getDouble()=0;

    virtual std::string getFormatString()=0;
};

}//namespace generic

#endif //_generic_Serializer_h_