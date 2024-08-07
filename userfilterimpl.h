#pragma once
#include  "userfilter.h"
#include "TypeComposition.h"
#include <string>
#include <vector>
#include <assert.h>
#include <algorithm>
#include <memory>

class UserFilterImpl
{
public:
	std::string name_;

	struct Port
	{
		std::string name;
		std::shared_ptr<MyObject> value;
	};

	struct Ports
	{
		std::vector<Port> ports_;
	
		std::string GetTypeByName(const char* name) const
		{
			return GetObjByName(name)->type;
		}

		std::string GetByName(const char* name) const
		{
			return ObjectToJSONString(GetObjByName(name));
		}

		std::shared_ptr<MyObject> GetObjByName(const char* name) const
		{
			const char* pDot = strstr(name, ".");

			if (pDot)
			{
				std::string subName(name, pDot);
				auto it = std::find_if(ports_.begin(), ports_.end(), [subName](Port const& p) {return p.name == subName; });
				assert(it != ports_.end());
				return it->value->GetByName(pDot + 1);
			}
			else
			{
				auto it = std::find_if(ports_.begin(), ports_.end(), [name](Port const& p) {return p.name == name; });
				assert(it != ports_.end());
				return it->value;
			}
		}

		void Add(const char* name, const char* type, const std::string& default_value)
		{
			AddObj(name, type, default_value.empty()? NewObject(type):ObjectFromJSONString(default_value.c_str()));
		}

		void AddObj(const char* name, const char* type, const std::shared_ptr<MyObject>& default_value)
		{
			assert(std::find_if(ports_.begin(), ports_.end(), [name](Port const& p) {return p.name == name; }) == ports_.end());
			if (default_value)
			{
				assert(type == default_value->type);
			}
			ports_.push_back(Port{ name, default_value ? default_value:NewObject(type) });
		}

		std::string Read(const char* name) const
		{
			return ObjectToJSONString(ReadObj(name));
		}
		std::shared_ptr<MyObject> ReadObj(const char* name) const
		{
			return GetObjByName(name);
		}

		void Write(const char* name, std::string const& t)
		{
			WriteObj(name, ObjectFromJSONString(t.c_str()));
		}
		void WriteObj(const char* name, std::shared_ptr<MyObject> const& t)
		{
			auto value = GetObjByName(name);
			assert(value->type == t->type);
			*value = *t;
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
