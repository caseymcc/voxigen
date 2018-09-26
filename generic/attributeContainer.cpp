#include "generic/attributeContainer.h"
#include "generic/attribute.h"

namespace generic
{
//bool AttributeContainer::hasAttribute(const std::string &name)
//{
//	return m_attributes.exists(name);
//}
AttributeContainer::AttributeContainer(const AttributeContainer &container)
{
    std::vector<std::string> keys=container.keys();

    for(size_t i=0; i<keys.size(); ++i)
    {
        addAttribute(container.attribute(keys[i]));
    }
}

std::vector<std::string> AttributeContainer::keys() const
{
    return m_attributes.keys();
}

bool AttributeContainer::exists(const std::string &name) const
{ return m_attributes.exists(name); }

SharedAttribute AttributeContainer::attribute(const std::string &name) const
{
    SharedAttribute attribute;

    attribute=m_attributes.find(name);
    if(attribute!=SharedAttribute())
        return m_attributes.at(name);
    return SharedAttribute();
}

//void AttributeContainer::setAttribute(std::string name, std::string value)
//{
//	SharedAttribute attribute;
//
//	attribute=m_attributes.find(name);
//	if(attribute != SharedAttribute())
//	{
//		attribute->fromString(value);
//		onAttributeChanged(name, attribute);
//	}
//}
//
//void AttributeContainer::setAttribute(std::string name, std::string value, const Strings &values)
//{
//	SharedAttribute attribute;
//
//	attribute=m_attributes.find(name);
//	if(attribute != SharedAttribute())
//	{
//		SharedAttributeStringEnum stringEnum=std::dynamic_pointer_cast<AttributeStringEnum>(attribute);
//
//		if(stringEnum != SharedAttributeStringEnum())
//		{
//			stringEnum->fromString(value);
//			stringEnum->setEnums(values);
//			onAttributeChanged(name, attribute);
//		}
//	}
//}

void AttributeContainer::addAttribute(SharedAttribute attribute)
{
    Attribute *newAttribute=NULL;
    switch(attribute->type())
    {
    case Type::Bool:
        newAttribute=new AttributeBool(this, *(AttributeBool *)attribute.get());
        break;
    case Type::Int:
        newAttribute=new AttributeInt(this, *(AttributeInt *)attribute.get());
        break;
    case Type::UInt:
        newAttribute=new AttributeUInt(this, *(AttributeUInt *)attribute.get());
        break;
    case Type::Int64:
        newAttribute=new AttributeInt64(this, *(AttributeInt64 *)attribute.get());
        break;
    case Type::UInt64:
        newAttribute=new AttributeUInt64(this, *(AttributeUInt64 *)attribute.get());
        break;
    case Type::Float:
        newAttribute=new AttributeFloat(this, *(AttributeFloat *)attribute.get());
        break;
    case Type::Double:
        newAttribute=new AttributeDouble(this, *(AttributeDouble *)attribute.get());
        break;
    case Type::String:
        newAttribute=new AttributeString(this, *(AttributeString *)attribute.get());
        break;
    case Type::String_Enum:
        newAttribute=new AttributeStringEnum(this, *(AttributeStringEnum *)attribute.get());
        break;
    }

    if(newAttribute!=NULL)
        m_attributes[attribute->name()].reset(newAttribute);
}

//void AttributeContainer::addAttribute(std::string name, bool value)
//{
//	m_attributes[name].reset(new AttributeBool(name, value));
//}
//
//void AttributeContainer::addAttribute(std::string name, int value)
//{
//	m_attributes[name].reset(new AttributeInt(name, value));
//}
//
//void AttributeContainer::addAttribute(std::string name, __int64 value)
//{
//	m_attributes[name].reset(new AttributeInt64(name, value));
//}
//
//void AttributeContainer::addAttribute(std::string name, float value)
//{
//	m_attributes[name].reset(new AttributeFloat(name, value));
//}
//
//void AttributeContainer::addAttribute(std::string name, double value)
//{
//	m_attributes[name].reset(new AttributeDouble(name, value));
//}
//
//void AttributeContainer::addAttribute(std::string name, char *value)
//{
//	m_attributes[name].reset(new AttributeString(name, value));
//}
//
//void AttributeContainer::addAttribute(std::string name, std::string value)
//{
//	m_attributes[name].reset(new AttributeString(name, value));
//}

//void AttributeContainer::addAttribute(std::string name, std::string value, const Strings &values)
//{
//	m_attributes[name].reset(new AttributeStringEnum(name, value, values));
//}

void AttributeContainer::attributeChanged(std::string name)
{
    SharedAttribute localAttribute=attribute(name);

    if(localAttribute)
    {
        onAttributeChanged(name, localAttribute);
        if(!m_changeListners.empty())
        {
            for(auto &listener:m_changeListners)
                listener->attributeChanged(this, name);
        }
    }
}

void AttributeContainer::serialize(Serializer *serializer)
{
    m_attributes.serialize(serializer);
}

void AttributeContainer::deserialize(Deserializer *deserializer)
{
    //	m_attributes.unserialize(deserializer);
    if(deserializer->key("attributes")) //need to fire off callbacks
    {
        if(deserializer->type()==Type::Object)
        {
            deserializer->openObject();

            std::vector<std::string> keys=m_attributes.keys();

            for(auto &pair:m_attributes)
            {
                SharedAttribute &attribute=pair.second;

                if(deserializer->key(attribute->name()))
                {
                    attribute->deserialize(deserializer);
                    onAttributeChanged(attribute->name(), attribute);
                }
            }

            deserializer->closeObject();
        }
    }

}

}//namespace generic