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

	bool open(const char *fileName, bool pretty=true);

	void startObject();
	void endObject();

	void endArray();
	void startArray();

	void addKey(const char * key);
	void addBool(const bool &value);
	void addString(const char *value);
	void addInt(const int &value);
	void addUInt(const unsigned int &value);
	void addInt64(const int64_t &value);
	void addUInt64(const uint64_t &value);
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

	bool open(const char *fileName);
    bool parse(const char *json);

	bool key(const char *key);
//	Type type();

    const char *name();

	bool openObject();
	void closeObject();

	bool openArray();
	void closeArray();

    bool advance();

	bool getBool();
    const char *getString();
	int getInt();
	unsigned int getUInt();
	int64_t getInt64();
	uint64_t getUInt64();
	float getFloat();
	double getDouble();

private:
    bool advanceIterator();
    bool advanceValueIterator();

	JsonUnserializerHidden *m_hidden;
};


}//namespace Limitless

#endif //_Attributes_h_