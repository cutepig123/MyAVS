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
