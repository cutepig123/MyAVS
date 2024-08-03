#include "UserFilter.h"
#include "UserFilterImpl.h"

#include <vector>
#include <string>
#include <map>
#include <assert.h>


UserFilter::UserFilter():impl_(new UserFilterImpl)
{

}

void UserFilter::SetName(const char* name)
{

}

void UserFilter::AddInput(const char* name, const char* type, const std::string& default_value)
{

}

void UserFilter::AddOutput(const char* name, const char* type, const std::string& default_value)
{

}

std::string UserFilter::ReadInput(const char* name)
{
	return std::string();
}

void UserFilter::WriteOutput(const char* name, std::string const& t)
{

}


std::map<std::string, std::function<UserFilter* ()>>& UserFilterCreator()
{
	static std::map<std::string, std::function<UserFilter* ()>>	filterCreators_;
	return filterCreators_;
}

void RegisterFilter(std::function<UserFilter* ()> const& create)
{
	std::unique_ptr<UserFilter> t(create());
	t->Define();
	//const char* name = t->Name();
	//assert(UserFilterCreator().find(name) == UserFilterCreator().end());
	//UserFilterCreator()[name] = create;
}

std::unique_ptr<UserFilter> CreateFilter(const char* name)
{
	auto it = UserFilterCreator().find(name);
	assert(it != UserFilterCreator().end());
	auto filter = it->second();
	filter->Define();
	return std::unique_ptr<UserFilter>(filter);
}

std::vector<std::string> GetAllUserFilters()
{
	std::vector<std::string> keys;
	for (const auto& pair : UserFilterCreator()) {
		keys.push_back(pair.first);
	}
	return keys;
}

