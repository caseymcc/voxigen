#include "generic/JsonSerializer.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"

#include <vector>
#include <stack>

namespace generic
{

struct JsonSerializerHidden
{
    JsonSerializerHidden():writer(nullptr), fileStream(nullptr), jsonFile(NULL) {}
    ~JsonSerializerHidden()
    {
        if(writer!=nullptr)
            delete writer;
        if(fileStream!=nullptr)
            delete fileStream;
        if(jsonFile!=NULL)
            fclose(jsonFile);
    }

    bool open(const std::string &fileName, bool pretty)
    {
        jsonFile=fopen(fileName.c_str(), "wb");

        if(jsonFile==NULL)
            return false;

        buffer.resize(65536);
        fileStream=new rapidjson::FileWriteStream(jsonFile, buffer.data(), buffer.size());

        //		if(pretty)
        writer=new rapidjson::PrettyWriter<rapidjson::FileWriteStream>(*fileStream);
        //		else
        //			writer=new rapidjson::Writer<rapidjson::FileWriteStream>(*fileStream);

        return true;
    }

    rapidjson::FileWriteStream *fileStream;
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> *writer;
    std::vector<char> buffer;
    FILE *jsonFile;
};

JsonSerializer::JsonSerializer()
{
    m_hidden=new JsonSerializerHidden;
}

JsonSerializer::~JsonSerializer()
{
    delete m_hidden;
}

bool JsonSerializer::open(std::string fileName, bool pretty)
{
    return m_hidden->open(fileName, pretty);
}

void JsonSerializer::startObject()
{
    m_hidden->writer->StartObject();
}

void JsonSerializer::endObject()
{
    m_hidden->writer->EndObject();
}

void JsonSerializer::startArray()
{
    m_hidden->writer->StartArray();
}
void JsonSerializer::endArray()
{
    m_hidden->writer->EndArray();
}

void JsonSerializer::addKey(const std::string key)
{
    m_hidden->writer->Key(key.c_str());
}

void JsonSerializer::addBool(const bool &value)
{
    m_hidden->writer->Bool(value);
}

void JsonSerializer::addString(const std::string &value)
{
    m_hidden->writer->String(value.c_str());
}

void JsonSerializer::addInt(const int &value)
{
    m_hidden->writer->Int(value);
}

void JsonSerializer::addUInt(const unsigned int &value)
{
    m_hidden->writer->Uint(value);
}

void JsonSerializer::addInt64(const __int64 &value)
{
    m_hidden->writer->Int64(value);
}

void JsonSerializer::addUInt64(const unsigned __int64 &value)
{
    m_hidden->writer->Uint64(value);
}

void JsonSerializer::addFloat(const float &value)
{
    m_hidden->writer->Double(value);
}

void JsonSerializer::addDouble(const double &value)
{
    m_hidden->writer->Double(value);
}


struct JsonDeserializerHidden
{
    JsonDeserializerHidden():fileStream(nullptr), jsonFile(NULL), currentValid(false), iteratorValid(false), valueIteratorValid(false) {}
    ~JsonDeserializerHidden() {}

    bool open(std::string fileName)
    {
        jsonFile=fopen(fileName.c_str(), "rb");

        if(jsonFile==NULL)
            return false;

        buffer.resize(65536);
        fileStream=new rapidjson::FileReadStream(jsonFile, buffer.data(), buffer.size());

        bool error=document.ParseStream(*fileStream).HasParseError();

        if(error)
            return false;

        iteratorValid=false;
        valueIteratorValid=false;
        currentValid=true;
        currentValue=&document;
        return true;
    }

    bool parse(std::string json)
    {
        bool error=document.Parse(json.c_str()).HasParseError();

        if(error)
            return false;

        iteratorValid=false;
        valueIteratorValid=false;
        currentValid=true;
        currentValue=&document;

        return true;
    }

    rapidjson::Document document;
    rapidjson::FileReadStream *fileStream;
    std::vector<char> buffer;
    FILE *jsonFile;

    std::stack<const rapidjson::Value *> valueStack;
    bool currentValid;
    const rapidjson::Value * currentValue;
    bool iteratorValid;
    rapidjson::Value::ConstMemberIterator currentIterator;
    std::stack<rapidjson::Value::ConstMemberIterator> iteratorStack;

    bool valueIteratorValid;
    rapidjson::Value::ConstValueIterator currentValueIterator;
    std::stack<rapidjson::Value::ConstValueIterator> valueIteratorStack;
};

JsonDeserializer::JsonDeserializer()
{
    m_hidden=new JsonDeserializerHidden();
}
JsonDeserializer::~JsonDeserializer()
{
    delete m_hidden;
}

bool JsonDeserializer::open(std::string fileName)
{
    return m_hidden->open(fileName);
}

bool JsonDeserializer::parse(std::string json)
{
    return m_hidden->parse(json);
}

bool JsonDeserializer::key(const std::string &key)
{
    if(m_hidden->iteratorValid)
    {
        m_hidden->currentIterator=m_hidden->currentValue->FindMember(key.c_str());

        if(m_hidden->currentIterator==m_hidden->currentValue->MemberEnd())
        {
            m_hidden->currentIterator=m_hidden->currentValue->MemberBegin();

            if(m_hidden->currentIterator==m_hidden->currentValue->MemberEnd())
                m_hidden->iteratorValid=false;
            return false;
        }
        m_hidden->iteratorValid=true;
        return true;
    }
    return false;
}

Type JsonDeserializer::type()
{
    rapidjson::Type type=(rapidjson::Type)7;//unknown

    if(m_hidden->iteratorValid)
        type=m_hidden->currentIterator->value.GetType();
    else if(m_hidden->valueIteratorValid)
        type=m_hidden->currentValueIterator->GetType();
    else if(m_hidden->currentValid)
        type=m_hidden->currentValue->GetType();

    switch(type)
    {
    case rapidjson::kNullType:
        return Type::Unknown;
    case rapidjson::kFalseType:
        return Type::Bool;
    case rapidjson::kTrueType:
        return Type::Bool;
    case rapidjson::kObjectType:
        return Type::Object;
    case rapidjson::kArrayType:
        return Type::Array;
    case rapidjson::kStringType:
        return Type::String;
    case rapidjson::kNumberType:
        return Type::Double;
    }
    return Type::Unknown;
}

std::string JsonDeserializer::name()
{
    if(m_hidden->iteratorValid)
    {
        return m_hidden->currentIterator->name.GetString();
    }
    //	else if(m_hidden->valueIteratorValid)
    //	{
    //		m_hidden->valueStack.top()->Get
    //		type=m_hidden->currentValueIterator->GetType();
    //	}
    return m_hidden->currentValue->GetString();
}

bool JsonDeserializer::openObject()
{
    if(m_hidden->iteratorValid && m_hidden->currentIterator->value.IsObject())
    {
        m_hidden->valueStack.push(m_hidden->currentValue);
        m_hidden->currentValue=&m_hidden->currentIterator->value;
        m_hidden->iteratorStack.push(m_hidden->currentIterator);
        m_hidden->iteratorValid=false;
        //		return true;
    }
    else if(m_hidden->valueIteratorValid)
    {
        m_hidden->valueStack.push(m_hidden->currentValue);
        m_hidden->currentValue=m_hidden->currentValueIterator;
        m_hidden->valueIteratorStack.push(m_hidden->currentValueIterator);
        m_hidden->valueIteratorValid=false;
        //		return true;
    }

    if(!m_hidden->currentValue->IsObject())
        return false;

    m_hidden->currentIterator=m_hidden->currentValue->MemberBegin();

    if(m_hidden->currentIterator==m_hidden->currentValue->MemberEnd())
    {
        m_hidden->iteratorValid=false;
        return false;
    }

    m_hidden->currentValid=false;
    m_hidden->iteratorValid=true;
    return true;
}
void JsonDeserializer::closeObject()
{
    assert(m_hidden->currentValue->IsObject());

    if(!m_hidden->valueStack.empty())
    {
        m_hidden->currentValue=m_hidden->valueStack.top();
        m_hidden->valueStack.pop();
        m_hidden->iteratorValid=false;

        if(m_hidden->currentValue->IsObject())
        {
            m_hidden->currentIterator=m_hidden->iteratorStack.top();
            m_hidden->iteratorStack.pop();
            advanceIterator();
        }
        else if(m_hidden->currentValue->IsArray())
        {
            m_hidden->currentValueIterator=m_hidden->valueIteratorStack.top();
            m_hidden->valueIteratorStack.pop();
            m_hidden->valueIteratorValid=true;
//            advanceValueIterator();
        }
        m_hidden->currentValid=true;
    }
}

bool JsonDeserializer::openArray()
{
    if(m_hidden->iteratorValid && m_hidden->currentIterator->value.IsArray())
    {
        m_hidden->valueStack.push(m_hidden->currentValue);
        m_hidden->currentValue=&m_hidden->currentIterator->value;
        m_hidden->iteratorStack.push(m_hidden->currentIterator);
        m_hidden->iteratorValid=false;
    }
    else if(m_hidden->valueIteratorValid)
    {
        m_hidden->valueStack.push(m_hidden->currentValue);
        m_hidden->currentValue=m_hidden->currentValueIterator;
        m_hidden->valueIteratorStack.push(m_hidden->currentValueIterator);
    }

    if(!m_hidden->currentValue->IsArray())
        return false;

    m_hidden->currentValueIterator=m_hidden->currentValue->Begin();
    if(m_hidden->currentValueIterator==m_hidden->currentValue->End())
        m_hidden->valueIteratorValid=false;
    else
    {
        m_hidden->valueIteratorValid=true;
        m_hidden->currentValid=false;
    }

    return true;
}

void JsonDeserializer::closeArray()
{
    assert(m_hidden->currentValue->IsArray());

    if(!m_hidden->valueStack.empty())
    {
        m_hidden->currentValue=m_hidden->valueStack.top();
        m_hidden->valueStack.pop();
        m_hidden->iteratorValid=false;

        if(m_hidden->currentValue->IsObject())
        {
            m_hidden->currentIterator=m_hidden->iteratorStack.top();
            m_hidden->iteratorStack.pop();
            advanceIterator();
        }
        else if(m_hidden->currentValue->IsArray())
        {
            m_hidden->currentValueIterator=m_hidden->valueIteratorStack.top();
            m_hidden->valueIteratorStack.pop();
            m_hidden->valueIteratorValid=true;
//            advanceValueIterator();
        }
        m_hidden->currentValid=false;
    }
}

bool JsonDeserializer::advance()
{
    if(m_hidden->iteratorValid)
        return advanceIterator();
    else if(m_hidden->valueIteratorValid)
        return advanceValueIterator();
    return false;
}

bool JsonDeserializer::getBool()
{
    if(m_hidden->iteratorValid)
        return m_hidden->currentIterator->value.GetBool();
    else if(m_hidden->valueIteratorValid)
    {
        bool value=m_hidden->currentValueIterator->GetBool();

        advanceValueIterator();
        return value;
    }
    return false;
}
std::string JsonDeserializer::getString()
{
    if(m_hidden->iteratorValid)
        return std::string(m_hidden->currentIterator->value.GetString());
    else if(m_hidden->valueIteratorValid)
    {
        std::string value=m_hidden->currentValueIterator->GetString();

        advanceValueIterator();
        return value;
    }
    return std::string();
}
int JsonDeserializer::getInt()
{
    if(m_hidden->iteratorValid)
        return m_hidden->currentIterator->value.GetInt();
    else if(m_hidden->valueIteratorValid)
    {
        int value=m_hidden->currentValueIterator->GetInt();

        advanceValueIterator();
        return value;
    }
    return -1;
}
unsigned int JsonDeserializer::getUInt()
{
    if(m_hidden->iteratorValid)
        return m_hidden->currentIterator->value.GetUint();
    else if(m_hidden->valueIteratorValid)
    {
        unsigned int value=m_hidden->currentValueIterator->GetUint();

        advanceValueIterator();
        return value;
    }
    return 0;
}

__int64 JsonDeserializer::getInt64()
{
    if(m_hidden->iteratorValid)
        return m_hidden->currentIterator->value.GetInt64();
    else if(m_hidden->valueIteratorValid)
    {
        __int64 value=m_hidden->currentValueIterator->GetInt64();

        advanceValueIterator();
        return value;
    }
    return -1;
}

unsigned __int64 JsonDeserializer::getUInt64()
{
    if(m_hidden->iteratorValid)
        return m_hidden->currentIterator->value.GetUint64();
    else if(m_hidden->valueIteratorValid)
    {
        unsigned __int64 value=m_hidden->currentValueIterator->GetUint64();

        advanceValueIterator();
        return value;
    }
    return 0;
}

float JsonDeserializer::getFloat()
{
    if(m_hidden->iteratorValid)
        return m_hidden->currentIterator->value.GetDouble();
    else if(m_hidden->valueIteratorValid)
    {
        double value=m_hidden->currentValueIterator->GetDouble();

        advanceValueIterator();
        return value;
    }
    return 0.0f;
}

double JsonDeserializer::getDouble()
{
    if(m_hidden->iteratorValid)
        return m_hidden->currentIterator->value.GetDouble();
    else if(m_hidden->valueIteratorValid)
    {
        double value=m_hidden->currentValueIterator->GetDouble();

        advanceValueIterator();
        return value;
    }
    return 0.0;
}

bool JsonDeserializer::advanceIterator()
{
    ++m_hidden->currentIterator;

    if(m_hidden->currentIterator==m_hidden->currentValue->MemberEnd())
        m_hidden->iteratorValid=false;
    else
        m_hidden->iteratorValid=true;

    return m_hidden->iteratorValid;
}

bool JsonDeserializer::advanceValueIterator()
{
    ++m_hidden->currentValueIterator;

    if(m_hidden->currentValueIterator==m_hidden->currentValue->End())
        m_hidden->valueIteratorValid=false;
    else
        m_hidden->valueIteratorValid=true;

    return m_hidden->valueIteratorValid;
}

std::string JsonDeserializer::getFormatString()
{
    if(m_hidden->iteratorValid)
    {
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

        m_hidden->currentIterator->value.Accept(writer);
        return sb.GetString();
    }
    return std::string();
}

}//namespace generic