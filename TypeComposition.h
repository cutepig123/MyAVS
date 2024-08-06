#pragma once
#include <set>
#include <string>

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

// class Object
// {
// 	Object* get(name);
// 	string type;
// };

// class NumObject:Object
// {
// 	double t;
// 	Object* get(name);//assert name==""
// 	string type;
// }

// class StringObject:Object
// {
// 	double t;
// 	Object* get(name);//assert name==""
// }

// class CompositionObject:Object
// {
// 	map<string, Object*> fiels;

// 	Object* get(name);
// 		Object* p=this;
// 		for x in name.split('.'):
// 			p = p->get(x)
// 		return p;
// 	string type;
// }
