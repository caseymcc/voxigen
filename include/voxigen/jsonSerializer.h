#ifndef _voxigen_jsonSerializer_h_
#define _voxigen_jsonSerializer_h_

#include "voxigen/voxigen_export.h"
#include <string>

///This is mainly hear to hide rapidjson's inclusion
///Using rapidjson interferes with at least Urho3d

namespace voxigen
{

struct JsonSerializerHidden;

class VOXIGEN_EXPORT JsonSerializer
{
public:
	JsonSerializer();
	~JsonSerializer();

	bool open(std::string fileName, bool pretty=true);

	void startObject();
	void endObject();

	void endArray();
	void startArray();

	void addKey(const std::string key);
	void addBool(const bool &value);
	void addString(const std::string &value);
	void addInt(const int &value);
	void addUInt(const unsigned int &value);
	void addInt64(const __int64 &value);
	void addUInt64(const unsigned __int64 &value);
	void addFloat(const float &value);
	void addDouble(const double &value);

private:
	JsonSerializerHidden *m_hidden;
};


struct JsonUnserializerHidden;

class VOXIGEN_EXPORT JsonUnserializer
{
public:
	JsonUnserializer();
	virtual ~JsonUnserializer();

	bool open(std::string fileName);
    bool parse(std::string json);

	bool key(const std::string &key);
//	Type type();

	std::string name();

	bool openObject();
	void closeObject();

	bool openArray();
	void closeArray();

    bool advance();

	bool getBool();
	std::string getString();
	int getInt();
	unsigned int getUInt();
	__int64 getInt64();
	unsigned __int64 getUInt64();
	float getFloat();
	double getDouble();

private:
    bool advanceIterator();
    bool advanceValueIterator();

	JsonUnserializerHidden *m_hidden;
};


}//namespace Limitless

#endif //_Attributes_h_