#ifndef _generic_attributeContainer_h_
#define _generic_attributeContainer_h_

#include "generic/generic_export.h"
#include "generic/attributes.h"
#include "generic/serializer.h"

#include <vector>
#include <memory>
#include <unordered_map>

namespace generic
{
class AttributeContainer;

class generic_EXPORT ChangeListener
{
public:
    ChangeListener() {}
    virtual ~ChangeListener() {}

    virtual void attributeChanged(AttributeContainer *parent, std::string name)=0;
};

template<typename _Listener>
class ChangeListnerPassthrough:public ChangeListener
{
public:
    ChangeListnerPassthrough(_Listener *listener):m_listener(listener) {}

    virtual void attributeChanged(AttributeContainer *parent, std::string name)
    {
        m_listener->attributeChanged(parent, name);
    }

private:
    _Listener *m_listener;
};


class generic_EXPORT AttributeContainer:public IChangeNotify
{
public:
	AttributeContainer(){}
	AttributeContainer(const AttributeContainer &container);
	~AttributeContainer() override{}

	std::vector<std::string> keys() const;
	bool exists(const std::string &name) const;
	SharedAttribute attribute(const std::string &name) const;
	
	template<typename T> void setAttribute(std::string name, T value)
	{
		SharedAttribute attribute;

		attribute=m_attributes.find(name);
		if(attribute != SharedAttribute())
		{
			attribute->from(value);
			onAttributeChanged(name, attribute);
		}
	}
	
	template<typename T> void setAttribute(std::string name, T value, const std::vector<T> &values);

	Attributes &attributes(){return m_attributes;}

	void addAttribute(SharedAttribute attribute);
	template <typename T> void addAttribute(std::string name, T value){m_attributes[name].reset(new AttributeTemplate<T>(this, name, value));}
	template <typename T> void addAttribute(std::string name, T value, const std::vector<T> &values){m_attributes[name].reset(new AttributeEnum<T>(this, name, value, values));}

	void attributeChanged(std::string name) override;

    void addChangeListener(ChangeListener *listener)
    {
        if(std::find(m_changeListners.begin(), m_changeListners.end(), listener) == m_changeListners.end())
            m_changeListners.push_back(listener);
    }
    void removeChangeListener(ChangeListener *listener)
    {
        auto iter=std::find(m_changeListners.begin(), m_changeListners.end(), listener);

        if(iter != m_changeListners.end())
            m_changeListners.erase(iter);
    }

	virtual void serialize(Serializer *serializer);
	virtual void deserialize(Deserializer *deserializer);
	
protected:
	virtual void onAttributeChanged(std::string name, SharedAttribute attribute){};

private:
	Attributes m_attributes;

    std::vector<ChangeListener *> m_changeListners;
};

template<> void AttributeContainer::setAttribute<const char *>(std::string name, const char *value)
{
	setAttribute(name, std::string(value));
}

template<> void AttributeContainer::addAttribute<const char *>(std::string name, const char * value){addAttribute(name, std::string(value));}
template<> void AttributeContainer::addAttribute<char *>(std::string name, char * value){addAttribute(name, std::string(value));}

}//namespace generic

#endif //__generic_attributeContainer_h_