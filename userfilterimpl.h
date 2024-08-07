#pragma once
#include  "userfilter.h"
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

	struct Ports
	{
		std::vector<Port> ports_;
	
		std::string GetTypeByName(const char* name) const
		{
			auto it = std::find_if(ports_.begin(), ports_.end(), [name](Port const& p) {return p.name == name; });
			assert(it != ports_.end());
			return it->type;
		}

		const Port& GetByName(const char* name) const
		{
			auto it = std::find_if(ports_.begin(), ports_.end(), [name](Port const& p) {return p.name == name; });
			assert(it != ports_.end());
			return *it;
		}

		size_t GetIndex(const char* name) const
		{
			auto it = std::find_if(ports_.begin(), ports_.end(), [name](Port const& p) {return p.name == name; });
			assert(it != ports_.end());
			return it - ports_.begin();
		}

		void Add(const char* name, const char* type, const std::string& default_value)
		{
			assert(std::find_if(ports_.begin(), ports_.end(), [name](Port const& p) {return p.name == name; }) == ports_.end());
			ports_.push_back(Port{ name, type, default_value });
		}

		std::string Read(const char* name) const
		{
			auto it = std::find_if(ports_.begin(), ports_.end(), [name](Port const& p) {return p.name == name; });
			assert(it != ports_.end());
			return it->value;
		}

		void Write(const char* name, std::string const& t)
		{
			auto it = std::find_if(ports_.begin(), ports_.end(), [name](Port const& p) {return p.name == name; });
			assert(it != ports_.end());
			it->value = t;
		}
	};

	Ports ins_;
	Ports outs_;

	UserFilterImpl()
	{

	}

	void SetName(const char* name)
	{
		name_ = name;
	}

	void AddInput(const char* name, const char* type, const std::string& default_value)
	{
		ins_.Add(name, type, default_value);
	}

	void AddOutput(const char* name, const char* type, const std::string& default_value)
	{
		outs_.Add(name, type, default_value);
	}

	std::string ReadInput(const char* name)
	{
		return ins_.Read(name);
	}

	void WriteOutput(const char* name, std::string const& t)
	{
		outs_.Write(name, t);
	}
};

struct Access
{
	static UserFilterImpl& GetUserFilterImpl(UserFilter& f)
	{
		return *f.impl_;
	}
	static const UserFilterImpl& GetUserFilterImpl(UserFilter const& f)
	{
		return *f.impl_;
	}
};
