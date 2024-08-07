#pragma once

#include "userfilter.h"
#include <string>

class MyAddUserFilter : public UserFilter
{
public:

	virtual void Define() override
	{
		SetName("Add");
		AddInput("a", "string", "1");
		AddInput("b", "int", "2");
		AddOutput("sum", "int", "0");
		AddOutput("sum_str", "string", "0");
	}
	virtual void Invoke() override
	{
		auto a = atoi(ReadInput("a").c_str());
		auto b= atoi(ReadInput("b").c_str());
		auto sum = a + b;
		char s[100];
		sprintf_s(s, "%d", sum);
		WriteOutput("sum", s);
		WriteOutput("sum_str", s);
	}
};

static int g_add = RegisterFilter([]()->UserFilter* {return new MyAddUserFilter; });
