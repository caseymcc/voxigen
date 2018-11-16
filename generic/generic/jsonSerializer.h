#ifndef _generic_JsonSerializer_h_
#define _generic_JsonSerializer_h_

#include "generic/generic_export.h"
#include "generic/serializer.h"

#include <string>

namespace generic
{

struct JsonSerializerHidden;

class GENERIC_EXPORT JsonSerializer:public Serializer
{
public:
    JsonSerializer();
    ~JsonSerializer() override;

    bool open(std::string fileName, bool pretty=true);

    void startObject() override;
    void endObject() override;

    void endArray() override;
    void startArray() override;

    void addKey(const std::string key) override;
    void addBool(const bool &value) override;
    void addString(const std::string &value) override;
    void addInt(const int &value) override;
    void addUInt(const unsigned int &value) override;
    void addInt64(const int64_t &value) override;
    void addUInt64(const uint64_t &value) override;
    void addFloat(const float &value) override;
    void addDouble(const double &value) override;

private:
    JsonSerializerHidden *m_hidden;
};


struct JsonDeserializerHidden;

class GENERIC_EXPORT JsonDeserializer:public Deserializer
{
public:
    JsonDeserializer();
    ~JsonDeserializer() override;

    bool open(std::string fileName);
    bool parse(std::string json);

    bool key(const std::string &key) override;
    Type type() override;

    std::string name() override;

    bool openObject() override;
    void closeObject() override;

    bool openArray() override;
    void closeArray() override;

    bool advance() override;

    bool getBool() override;
    std::string getString() override;
    int getInt() override;
    unsigned int getUInt() override;
    int64_t getInt64() override;
    uint64_t getUInt64() override;
    float getFloat() override;
    double getDouble() override;

    std::string getFormatString() override;

private:
    bool advanceIterator();
    bool advanceValueIterator();

    JsonDeserializerHidden *m_hidden;
};


}//namespace generic

#endif //_generic_JsonSerializer_h_