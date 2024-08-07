#pragma once
#include "TypeComposition.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class UserFilterImpl;

class UserFilter
{
	friend struct Access;
	std::unique_ptr<UserFilterImpl> impl_;
public:
	UserFilter();
	virtual ~UserFilter();

	virtual void Define() = 0;
	virtual void Invoke() = 0;

protected:
	// For derived class to call
	void SetName(const char* name);

	void AddInput(const char* name, const char* type, const std::string& default_value);

	void AddOutput(const char* name, const char* type, const std::string& default_value);

	std::string ReadInput(const char* name);
	std::shared_ptr<MyObject> ReadInput2(const char* name);

	void WriteOutput(const char* name, std::string const& t);
	void WriteOutput2(const char* name, std::shared_ptr<MyObject> const& t);
private:

};

int RegisterFilter(std::function<UserFilter*()> const& create);
std::unique_ptr<UserFilter> CreateFilter(const char* name);
std::vector<std::string> GetAllUserFilters();


