#ifndef _generic_attribute_h_
#define _generic_attribute_h_

#include "generic/types.h"
#include "generic/serializer.h"

#include <assert.h>
#include <memory>
#include <unordered_map>

#pragma warning(push)
#pragma warning(disable:4800)
#include <boost/lexical_cast.hpp>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable:4251)

//#ifdef GENERIC_USE_BOOST_VARIANT
//typedef boost::variant<std::string, int, float, double> Attribute;
//#else
//typedef std::variant<std::string, int, float, double> Attribute;
//#endif
//typedef std::unordered_map<std::string, Attribute> Attributes;

template<typename classType> class AttributeTemplate;

namespace generic
{

class GENERIC_EXPORT IChangeNotify
{
public:
	IChangeNotify(){}
	virtual ~IChangeNotify(){}

	virtual void attributeChanged(std::string name)=0;
};

class Attribute;

typedef std::shared_ptr<Attribute> SharedAttribute;
typedef std::vector<SharedAttribute> SharedAttributes;

class GENERIC_EXPORT Attribute
{
public:
	Attribute(Type type, std::string name):m_type(type), m_name(name){}
	virtual ~Attribute(){};

	Type type(){return m_type;}
	std::string name(){return m_name;}

	template <typename T> void from(T value);
	template <typename T> T to() const;

	virtual void fromString(std::string value)=0;
	virtual void fromInt(int value)=0;
	virtual void fromUInt(unsigned int value)=0;
	virtual void fromInt64(int64_t value)=0;
	virtual void fromUInt64(uint64_t value)=0;
	virtual void fromFloat(float value)=0;
	virtual void fromDouble(double value)=0;
	virtual void fromBool(bool value)=0;

	virtual std::string toString() const=0;
	virtual int toInt() const=0;
	virtual unsigned int toUInt() const=0;
	virtual int64_t toInt64() const=0;
	virtual uint64_t toUInt64() const=0;
	virtual float toFloat() const=0;
	virtual double toDouble() const=0;
	virtual bool toBool() const=0;

	virtual SharedAttributes values(){return SharedAttributes();}

//	template<typename T> Attribute &operator=(const T &value) { from(value); return *this; }

	virtual bool operator==(const Attribute &attribute) const{return (toString()==attribute.toString());}
	virtual bool equal(std::shared_ptr<Attribute> attribute) const{return (toString()==attribute->toString());}

	virtual void serialize(Serializer *serializer)=0;
	virtual void deserialize(Deserializer *serializer)=0;

protected:
	Type m_type;
	std::string m_name;
};
typedef std::unordered_map<std::string, SharedAttribute> SharedAttributeMap;

template<typename classType>
class AttributeTemplate:public Attribute
{
public:
	AttributeTemplate(const AttributeTemplate<classType> &attribute):Attribute(attribute.m_type, attribute.m_name), m_parent(nullptr) { m_value=attribute.m_value; };
    AttributeTemplate(IChangeNotify *parent, const AttributeTemplate<classType> &attribute):Attribute(attribute.m_type, attribute.m_name), m_parent(parent) { m_value=attribute.m_value; };

///////////////////////////////////////////////////////////////////////////////
//if you are here it is because Attribute does not know how to handle your type
    AttributeTemplate(const std::string name):Attribute(Type::Unknown, name), m_parent(nullptr) { assert(false); };
	AttributeTemplate(const std::string name, classType value):Attribute(Type::Unknown, name), m_parent(nullptr), m_value(value) { assert(false); };
	AttributeTemplate(IChangeNotify *parent, const std::string name):Attribute(Type::Unknown, name), m_parent(parent){assert(false);};
	AttributeTemplate(IChangeNotify *parent, const std::string name, classType value):Attribute(Type::Unknown, name), m_parent(parent), m_value(value){assert(false);};
///////////////////////////////////////////////////////////////////////////////

	~AttributeTemplate() override {};

	classType &value(){return m_value;}

	template<typename T> void from(const T &value){m_value=boost::lexical_cast<classType>(value);}
	template<typename T> void to() const{boost::lexical_cast<T>(m_value);}

	void fromString(std::string value) override{m_value=boost::lexical_cast<classType>(value); if(m_parent != nullptr) m_parent->attributeChanged(m_name);}
	void fromInt(int value) override {m_value=boost::lexical_cast<classType>(value); if(m_parent != nullptr) m_parent->attributeChanged(m_name);}
	void fromUInt(unsigned int value) override {m_value=boost::lexical_cast<classType>(value); if(m_parent != nullptr) m_parent->attributeChanged(m_name);}
	void fromInt64(int64_t value) override {m_value=boost::lexical_cast<classType>(value); if(m_parent != nullptr) m_parent->attributeChanged(m_name);}
	void fromUInt64(uint64_t value) override {m_value=boost::lexical_cast<classType>(value); if(m_parent != nullptr) m_parent->attributeChanged(m_name);}
	void fromFloat(float value) override {m_value=boost::lexical_cast<classType>(value); if(m_parent != nullptr) m_parent->attributeChanged(m_name);}
	void fromDouble(double value) override {m_value=boost::lexical_cast<classType>(value); if(m_parent != nullptr) m_parent->attributeChanged(m_name);}
	void fromBool(bool value) override {m_value=boost::lexical_cast<classType>(value); if(m_parent != nullptr) m_parent->attributeChanged(m_name);}

	std::string toString() const override {return boost::lexical_cast<std::string>(m_value);}
	int toInt() const override {return boost::lexical_cast<int>(m_value);}
	unsigned int toUInt() const override {return boost::lexical_cast<int>(m_value);}
	int64_t toInt64() const override {return boost::lexical_cast<int64_t>(m_value);}
	uint64_t toUInt64() const override {return boost::lexical_cast<int64_t>(m_value);}
	float toFloat() const override {return boost::lexical_cast<float>(m_value);}
	double toDouble() const override {return boost::lexical_cast<double>(m_value);}
	bool toBool() const override {return boost::lexical_cast<bool>(m_value);}

	void serialize(Serializer *serializer)  override { serializer->addKey(name()); serializer->addString(toString()); }
	void deserialize(Deserializer *deserializer)  override { if(deserializer->key(name())) fromString(deserializer->getString()); }

protected:
	classType m_value;
	IChangeNotify *m_parent;
};

template <typename T> void Attribute::from(T value){AttributeTemplate<T> *d=(AttributeTemplate<T> *)(this);d?d->template from<T>(value):assert(false);}
template <typename T> T Attribute::to() const { const AttributeTemplate<T> *d=(const AttributeTemplate<T> *)(this); return d?d->template to<T>():assert(false); }

template<> GENERIC_EXPORT AttributeTemplate<bool>::AttributeTemplate(const std::string name):Attribute(Type::Bool, name), m_parent(nullptr), m_value(false) {};
template<> GENERIC_EXPORT AttributeTemplate<bool>::AttributeTemplate(const std::string name, bool value):Attribute(Type::Bool, name), m_parent(nullptr), m_value(value) {};
template<> GENERIC_EXPORT AttributeTemplate<bool>::AttributeTemplate(IChangeNotify *parent, const std::string name):Attribute(Type::Bool, name), m_parent(parent), m_value(false){};
template<> GENERIC_EXPORT AttributeTemplate<bool>::AttributeTemplate(IChangeNotify *parent, const std::string name, bool value):Attribute(Type::Bool, name), m_parent(parent), m_value(value){};
template<> void GENERIC_EXPORT AttributeTemplate<bool>::serialize(Serializer *serializer) { serializer->addKey(name()); serializer->addBool(m_value); }
template<> void GENERIC_EXPORT AttributeTemplate<bool>::deserialize(Deserializer *serializer) { if(serializer->key(name())) m_value=serializer->getBool(); }
typedef AttributeTemplate<bool> AttributeBool;

template<> GENERIC_EXPORT AttributeTemplate<int>::AttributeTemplate(const std::string name):Attribute(Type::Int, name), m_parent(nullptr), m_value(0) {};
template<> GENERIC_EXPORT AttributeTemplate<int>::AttributeTemplate(const std::string name, int value):Attribute(Type::Int, name), m_parent(nullptr), m_value(value) {};
template<> GENERIC_EXPORT AttributeTemplate<int>::AttributeTemplate(IChangeNotify *parent, const std::string name):Attribute(Type::Int, name), m_parent(parent), m_value(0){};
template<> GENERIC_EXPORT AttributeTemplate<int>::AttributeTemplate(IChangeNotify *parent, const std::string name, int value):Attribute(Type::Int, name), m_parent(parent), m_value(value){};
template<> void GENERIC_EXPORT AttributeTemplate<int>::serialize(Serializer *serializer) { serializer->addKey(name()); serializer->addInt(m_value); }
template<> void GENERIC_EXPORT AttributeTemplate<int>::deserialize(Deserializer *serializer) { if(serializer->key(name())) m_value=serializer->getInt(); }
typedef AttributeTemplate<int> AttributeInt;

template<> GENERIC_EXPORT AttributeTemplate<unsigned int>::AttributeTemplate(const std::string name):Attribute(Type::UInt, name), m_parent(nullptr), m_value(0) {};
template<> GENERIC_EXPORT AttributeTemplate<unsigned int>::AttributeTemplate(const std::string name, unsigned int value):Attribute(Type::UInt, name), m_parent(nullptr), m_value(value) {};
template<> GENERIC_EXPORT AttributeTemplate<unsigned int>::AttributeTemplate(IChangeNotify *parent, const std::string name):Attribute(Type::UInt, name), m_parent(parent), m_value(0){};
template<> GENERIC_EXPORT AttributeTemplate<unsigned int>::AttributeTemplate(IChangeNotify *parent, const std::string name, unsigned int value):Attribute(Type::UInt, name), m_parent(parent), m_value(value){};
template<> void GENERIC_EXPORT AttributeTemplate<unsigned int>::serialize(Serializer *serializer) { serializer->addKey(name()); serializer->addUInt(m_value); }
template<> void GENERIC_EXPORT AttributeTemplate<unsigned int>::deserialize(Deserializer *serializer) { if(serializer->key(name())) m_value=serializer->getUInt(); }
typedef AttributeTemplate<unsigned int> AttributeUInt;

template<> GENERIC_EXPORT AttributeTemplate<int64_t>::AttributeTemplate(const std::string name):Attribute(Type::Int64, name), m_parent(nullptr), m_value(0) {};
template<> GENERIC_EXPORT AttributeTemplate<int64_t>::AttributeTemplate(const std::string name, int64_t value):Attribute(Type::Int64, name), m_parent(nullptr), m_value(value) {};
template<> GENERIC_EXPORT AttributeTemplate<int64_t>::AttributeTemplate(IChangeNotify *parent, const std::string name):Attribute(Type::Int64, name), m_parent(parent), m_value(0){};
template<> GENERIC_EXPORT AttributeTemplate<int64_t>::AttributeTemplate(IChangeNotify *parent, const std::string name, int64_t value):Attribute(Type::Int64, name), m_parent(parent), m_value(value){};
template<> void GENERIC_EXPORT AttributeTemplate<int64_t>::serialize(Serializer *serializer) { serializer->addKey(name()); serializer->addInt64(m_value); }
template<> void GENERIC_EXPORT AttributeTemplate<int64_t>::deserialize(Deserializer *serializer) { if(serializer->key(name())) m_value=serializer->getInt(); }
typedef AttributeTemplate<int64_t> AttributeInt64;

template<> GENERIC_EXPORT AttributeTemplate<uint64_t>::AttributeTemplate(const std::string name):Attribute(Type::UInt64, name), m_parent(nullptr), m_value(0) {};
template<> GENERIC_EXPORT AttributeTemplate<uint64_t>::AttributeTemplate(const std::string name, uint64_t value):Attribute(Type::UInt64, name), m_parent(nullptr), m_value(value) {};
template<> GENERIC_EXPORT AttributeTemplate<uint64_t>::AttributeTemplate(IChangeNotify *parent, const std::string name):Attribute(Type::UInt64, name), m_parent(parent), m_value(0){};
template<> GENERIC_EXPORT AttributeTemplate<uint64_t>::AttributeTemplate(IChangeNotify *parent, const std::string name, uint64_t value):Attribute(Type::UInt64, name), m_parent(parent), m_value(value){};
template<> void GENERIC_EXPORT AttributeTemplate<uint64_t>::serialize(Serializer *serializer) { serializer->addKey(name()); serializer->addUInt64(m_value); }
template<> void GENERIC_EXPORT AttributeTemplate<uint64_t>::deserialize(Deserializer *serializer) { if(serializer->key(name())) m_value=serializer->getUInt(); }
typedef AttributeTemplate<uint64_t> AttributeUInt64;

template<> GENERIC_EXPORT AttributeTemplate<float>::AttributeTemplate(const std::string name):Attribute(Type::Float, name), m_parent(nullptr), m_value(0.0f) {};
template<> GENERIC_EXPORT AttributeTemplate<float>::AttributeTemplate(const std::string name, float value):Attribute(Type::Float, name), m_parent(nullptr), m_value(value) {};
template<> GENERIC_EXPORT AttributeTemplate<float>::AttributeTemplate(IChangeNotify *parent, const std::string name):Attribute(Type::Float, name), m_parent(parent), m_value(0.0f){};
template<> GENERIC_EXPORT AttributeTemplate<float>::AttributeTemplate(IChangeNotify *parent, const std::string name, float value):Attribute(Type::Float, name), m_parent(parent), m_value(value){};
template<> void GENERIC_EXPORT AttributeTemplate<float>::serialize(Serializer *serializer) { serializer->addKey(name()); serializer->addFloat(m_value); }
template<> void GENERIC_EXPORT AttributeTemplate<float>::deserialize(Deserializer *serializer) { if(serializer->key(name())) m_value=serializer->getFloat(); }
typedef AttributeTemplate<float> AttributeFloat;

template<> GENERIC_EXPORT AttributeTemplate<double>::AttributeTemplate(const std::string name):Attribute(Type::Double, name), m_parent(nullptr), m_value(0.0) {};
template<> GENERIC_EXPORT AttributeTemplate<double>::AttributeTemplate(const std::string name, double value):Attribute(Type::Double, name), m_parent(nullptr), m_value(value) {};
template<> GENERIC_EXPORT AttributeTemplate<double>::AttributeTemplate(IChangeNotify *parent, const std::string name):Attribute(Type::Double, name), m_parent(parent), m_value(0.0){};
template<> GENERIC_EXPORT AttributeTemplate<double>::AttributeTemplate(IChangeNotify *parent, const std::string name, double value):Attribute(Type::Double, name), m_parent(parent), m_value(value){};
template<> void GENERIC_EXPORT AttributeTemplate<double>::serialize(Serializer *serializer) { serializer->addKey(name()); serializer->addDouble(m_value); }
template<> void GENERIC_EXPORT AttributeTemplate<double>::deserialize(Deserializer *serializer) { if(serializer->key(name())) m_value=serializer->getDouble(); }
typedef AttributeTemplate<double> AttributeDouble;

template<> GENERIC_EXPORT AttributeTemplate<std::string>::AttributeTemplate(const std::string name):Attribute(Type::String, name), m_parent(nullptr), m_value("") {};
template<> GENERIC_EXPORT AttributeTemplate<std::string>::AttributeTemplate(const std::string name, std::string value):Attribute(Type::String, name), m_parent(nullptr), m_value(value) {};
template<> GENERIC_EXPORT AttributeTemplate<std::string>::AttributeTemplate(IChangeNotify *parent, const std::string name):Attribute(Type::String, name), m_parent(parent), m_value(""){};
template<> GENERIC_EXPORT AttributeTemplate<std::string>::AttributeTemplate(IChangeNotify *parent, const std::string name, std::string value):Attribute(Type::String, name), m_parent(parent), m_value(value){};
template<> GENERIC_EXPORT void AttributeTemplate<std::string>::serialize(Serializer *serializer) { serializer->addKey(name()); serializer->addString(m_value); }
template<> void GENERIC_EXPORT AttributeTemplate<std::string>::deserialize(Deserializer *serializer) { if(serializer->key(name())) m_value=serializer->getString(); }
typedef AttributeTemplate<std::string> AttributeString;

template<typename classType>
class GENERIC_EXPORT AttributeEnum:public AttributeTemplate<classType>
{
public:
	AttributeEnum(const std::string name):AttributeTemplate<classType>(nullptr, name) {};
	AttributeEnum(const std::string name, const classType value):AttributeTemplate<classType>(nullptr, name, value) {};
	AttributeEnum(const std::string name, const classType  value, const std::vector<classType> values):AttributeTemplate<classType>(nullptr, name, value), m_values(values) {};
	AttributeEnum(IChangeNotify *parent, const std::string name):AttributeTemplate<classType>(parent, name){};
	AttributeEnum(IChangeNotify *parent, const std::string name, const classType value):AttributeTemplate<classType>(parent, name, value){};
	AttributeEnum(IChangeNotify *parent, const std::string name, const classType  value, const std::vector<classType> values):AttributeTemplate<classType>(parent, name, value), m_values(values){};
	AttributeEnum(const AttributeEnum<classType> &attribute):AttributeTemplate<classType>(nullptr, attribute.m_name, attribute.m_value){m_values=attribute.m_values;}
	AttributeEnum(IChangeNotify *parent, const AttributeEnum<classType> &attribute):AttributeTemplate<classType>(parent, attribute.m_name, attribute.m_value){m_values=attribute.m_values;}
	~AttributeEnum() override {};

	std::vector<classType> enums() const{return m_values;}
	void setEnums(std::vector<classType> enumValues){m_values=enumValues;}

	virtual SharedAttributes values()
	{
		SharedAttributes values;

		for(size_t i=0; i<m_values.size(); ++i)
		{
			SharedAttribute attribute(new AttributeTemplate<classType>(nullptr, "enum", m_values[i]));
			
			values.push_back(attribute);
		}

		return values;
	}

protected:
	std::vector<classType> m_values;
};

//template AttributeEnum<int>;
typedef AttributeEnum<int> AttributeIntEnum;

//template AttributeEnum<float>;
typedef AttributeEnum<float> AttributeFloatEnum;

//template AttributeEnum<std::string>;
typedef AttributeEnum<std::string> AttributeStringEnum;
typedef std::shared_ptr<AttributeStringEnum> SharedAttributeStringEnum;

}//namespace generic

#endif //_generic_attribute_h_