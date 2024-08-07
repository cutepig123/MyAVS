#pragma once
#include "TypeComposition.h"
#include "userfilter.h"
#include <string>

class MyAddUserFilter : public UserFilter
{
public:

	virtual void Define() override
	{
		SetName("Add");
		AddInput("a", "Window", "");
		AddInput("b", "string", "");
		AddOutput("length", "int", "");
		AddOutput("sum_str", "string", "");
	}
	
	virtual void Invoke() override
	{
		// TODO: Ugly Code
		auto a = ReadInput("a");
		auto b= ReadInput("b");
		
		{
			auto sum = strlen(a.c_str()) + strlen(b.c_str());
			char sum_str[100];
			sprintf_s(sum_str, "%d", sum);
			WriteOutput2("length", NewIntObject(sum_str));
		}

		{
			std::string sum_str;
			sum_str += a;
			sum_str += b;
			WriteOutput2("sum_str", NewStringObject(sum_str.c_str()));
		}
	}
};

static int g_add = RegisterFilter([]()->UserFilter* {return new MyAddUserFilter; });
