#pragma once
#include <string>
#include <vector>
#include <assert.h>
#include <algorithm>

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

	void SetName(const char* name)
	{
		name_ = name;
	}

	static void Add(std::vector<Port>& ins_, const char* name, const char* type, const std::string& default_value)
	{
		assert(std::find_if(ins_.begin(), ins_.end(), [name](Port const& p) {return p.name == name; }) == ins_.end());
		ins_.push_back(Port{ name, type, default_value });
	}

	void AddInput(const char* name, const char* type, const std::string& default_value)
	{
		Add(ins_, name, type, default_value);
	}

	void AddOutput(const char* name, const char* type, const std::string& default_value)
	{
		Add(outs_, name, type, default_value);
	}

	static std::string Read(std::vector<Port>& ins_, const char* name)
	{
		auto it = std::find_if(ins_.begin(), ins_.end(), [name](Port const& p) {return p.name == name; });
		assert(it != ins_.end());
		return it->value;
	}

	std::string ReadInput(const char* name)
	{
		return Read(ins_, name);
	}

	static void Write(std::vector<Port>& outs_, const char* name, std::string const& t)
	{
		auto it = std::find_if(outs_.begin(), outs_.end(), [name](Port const& p) {return p.name == name; });
		assert(it != outs_.end());
		it->value = t;
	}

	void WriteOutput(const char* name, std::string const& t)
	{
		Write(outs_, name, t);
	}
};

struct Access
{
	static UserFilterImpl& GetUserFilterImpl(UserFilter& f)
	{
		return *f.impl_;
	}
};
