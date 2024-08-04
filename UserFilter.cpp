#include "UserFilter.h"
#include "UserFilterImpl.h"

#include <vector>
#include <string>
#include <map>
#include <assert.h>

static std::map<std::string, std::function<UserFilter* ()>>	filterCreators_;

UserFilter::UserFilter():impl_(new UserFilterImpl)
{

}

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


struct Access
{
	static UserFilterImpl& GetUserFilterImpl(UserFilter& f)
	{
		return *f.impl_;
	}
};

void RegisterFilter(std::function<UserFilter* ()> const& create)
{
	std::unique_ptr<UserFilter> t(create());
	t->Define();
	auto name = Access::GetUserFilterImpl(*t).name_;
	assert(filterCreators_.find(name) == filterCreators_.end());
	filterCreators_[name] = create;
}

std::unique_ptr<UserFilter> CreateFilter(const char* name)
{
	auto it = filterCreators_.find(name);
	assert(it != filterCreators_.end());
	auto filter = it->second();
	filter->Define();
	return std::unique_ptr<UserFilter>(filter);
}

std::vector<std::string> GetAllUserFilters()
{
	std::vector<std::string> keys;
	for (const auto& pair : filterCreators_) {
		keys.push_back(pair.first);
	}
	return keys;
}

