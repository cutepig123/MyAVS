#pragma once
#include <set>
#include <string>
#include <map>
#include <boost/variant.hpp>
#include <memory>

// Model Type and Instance/Object
// TODO: ??

struct Field
{
	std::string type;
	std::string name;

	bool operator<(const Field& rhs) const
	{
		if (type < rhs.type)
			return true;
		if (name < rhs.name)
			return true;
		return false;
	}
};

int RegisterType(const char* name, std::set<std::string> const& children);
const std::set<Field>& GetTypeItems(const char* name);

struct MyObject : public std::enable_shared_from_this<MyObject>
{
	std::string type;
	
	struct Struct
	{
		typedef std::map<std::string, std::shared_ptr<MyObject>> fields_type;
		fields_type fields;
	};
	
	typedef boost::variant<std::string, double, Struct> value_type;
	value_type value;

	// name: a.b.c
	// if name is empty, returns current object
	// otherwise return a child item
	std::shared_ptr<MyObject> GetByName(const char* name)
	{
		if (!name || !name[0]) return shared_from_this();

		const Struct *pStruct = boost::get<Struct>(&value);
		const char* pDot = strstr(name, ".");
		if (pDot)
		{
			std::string subName(name, pDot);
			auto subObj = pStruct->fields.at(subName);
			return subObj->GetByName(pDot + 1);
		}
		else
		{
			auto subObj = pStruct->fields.at(name);
			return subObj;
		}
	}
};

std::shared_ptr<MyObject> NewObject(const char* type);
std::shared_ptr<MyObject>	ObjectFromJSONString(const char* s);
std::string ObjectToJSONString(std::shared_ptr<MyObject> const& o);
std::shared_ptr<MyObject> NewStringObject(const char* value);
std::string CheckStringObject(std::shared_ptr<MyObject> const& o);
std::shared_ptr<MyObject> NewIntObject(const char* value);
std::string CheckIntObject(std::shared_ptr<MyObject> const& o);
