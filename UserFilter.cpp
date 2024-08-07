#include "UserFilter.h"
#include "UserFilterImpl.h"

#include <vector>
#include <string>
#include <map>
#include <assert.h>

static std::map<std::string, std::function<UserFilter* ()>>	*g_pfilterCreators_ = nullptr;

std::map<std::string, std::function<UserFilter* ()>>& GetFilterCreatorFactory()
{
	static std::map<std::string, std::function<UserFilter* ()>>	filterCreators_;
	if (!g_pfilterCreators_)
	{
		g_pfilterCreators_ = &filterCreators_;
	}
	return filterCreators_;
}

UserFilter::UserFilter():impl_(new UserFilterImpl)
{

}

UserFilter::~UserFilter()
{}

void UserFilter::SetName(const char* name)
{
	impl_->SetName(name);
}

void UserFilter::AddInput(const char* name, const char* type, const std::string& default_value)
{
	impl_->AddInput(name, type, default_value);
}

void UserFilter::AddOutput(const char* name, const char* type, const std::string& default_value)
{
	impl_->AddOutput(name, type, default_value);
}

std::string UserFilter::ReadInput(const char* name)
{
	return impl_->ReadInput(name);
}

void UserFilter::WriteOutput(const char* name, std::string const& t)
{
	impl_->WriteOutput(name, t);
}

std::shared_ptr<MyObject> UserFilter::ReadInput2(const char* name)
{
	return impl_->ins_.ReadObj(name);
}

void UserFilter::WriteOutput2(const char* name, std::shared_ptr<MyObject> const& t)
{
	impl_->outs_.WriteObj(name, t);
}

int RegisterFilter(std::function<UserFilter* ()> const& create)
{
	std::unique_ptr<UserFilter> t(create());
	t->Define();
	auto name = Access::GetUserFilterImpl(*t).name_;
	assert(GetFilterCreatorFactory().find(name) == GetFilterCreatorFactory().end());
	GetFilterCreatorFactory()[name] = create;
	return 0;
}

std::unique_ptr<UserFilter> CreateFilter(const char* name)
{
	auto it = GetFilterCreatorFactory().find(name);
	assert(it != GetFilterCreatorFactory().end());
	auto filter = it->second();
	filter->Define();
	return std::unique_ptr<UserFilter>(filter);
}

std::vector<std::string> GetAllUserFilters()
{
	std::vector<std::string> keys;
	for (const auto& pair : GetFilterCreatorFactory()) {
		keys.push_back(pair.first);
	}
	return keys;
}

