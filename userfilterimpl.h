#pragma once
#include <string>
#include <vector>

class UserFilterImpl
{
public:
	std::string name_;

	struct Port
	{
		std::string name;
		std::string type;
		std::string value;
	};
	std::vector<Port> ins_;
	std::vector<Port> outs_;

	UserFilterImpl()
	{

	}
};
