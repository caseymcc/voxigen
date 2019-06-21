#ifndef _generic_attributes_h_
#define _generic_attributes_h_

#ifndef GENERIC_DISABLE_ATTRIBUTE

#include "generic/generic_export.h"
#include "generic/attribute.h"

#include <memory>
#include <unordered_map>

#pragma warning(push)
#pragma warning(disable:4800)
#include <boost/lexical_cast.hpp>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable:4251)

namespace generic
{

class generic_EXPORT Attributes
{
public:
	void addAttribute(SharedAttribute attribute);
	template <typename T> void addAttribute(std::string name, T value) { m_attributes[name].reset(new AttributeTemplate<T>(name, value)); }

	typedef SharedAttributeMap::value_type value_type;
	typedef SharedAttributeMap::iterator iterator;
	typedef SharedAttributeMap::const_iterator const_iterator;

	size_t size() const{return m_attributes.size();}
	std::vector<std::string> keys() const;
	bool exists(std::string name) const{return (m_attributes.find(name) != m_attributes.end());}
	SharedAttribute find(const char *key) const;
	SharedAttribute find(const std::string &key) const;
	SharedAttribute &operator[](const char *key){const std::string constKey(key); return m_attributes[constKey];}
	SharedAttribute &operator[](const std::string &key){return m_attributes[key];}
	SharedAttribute const &operator[](const char *key) const { const std::string constKey(key); return m_attributes.at(constKey); }
	SharedAttribute const &operator[](const std::string &key) const { return m_attributes.at(key); }

	SharedAttribute const&at(const std::string key) const{return m_attributes.at(key);}

	iterator begin(){return m_attributes.begin();}
    const_iterator begin() const{return m_attributes.begin();}
    iterator end(){return m_attributes.end();}
    const_iterator end() const{return m_attributes.end();}

	iterator erase(const_iterator position){return m_attributes.erase(position);}

	void serialize(Serializer *serializer);
	void deserialize(Deserializer *serializer);

private:
	SharedAttributeMap m_attributes;
};

#pragma warning(pop)

void generic_EXPORT merge(Attributes &dst, const Attributes &src);

}//namespace generic

#endif //GENERIC_DISABLE_ATTRIBUTE
#endif //_generic_attributes_h_