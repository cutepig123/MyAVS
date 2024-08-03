#include "UserFilter.h"

#include <boost/function.hpp>
#include <boost/serialization/export.hpp>
// Note: Must include archive for BOOST_CLASS_EXPORT. Refer https://www.boost.org/doc/libs/1_82_0/libs/serialization/doc/special.html#export
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <vector>
#include <string>
#include <map>

std::map<std::string, boost::function<UserFilter*()>>&	UserFilterCreator()
{
	static std::map<std::string, boost::function<UserFilter*()>>	filterCreators_;
	return filterCreators_;
}

int RegisterFilter(boost::function<UserFilter*()> const& create)
{
	boost::scoped_ptr<UserFilter> t(create());
	t->Define();
	const char* name = t->Name();
	assert(UserFilterCreator().find(name) == UserFilterCreator().end());
	UserFilterCreator()[name] = create;
	return 0;
}

std::unique_ptr<UserFilter> CreateFilter(const char* name)
{
	auto it = UserFilterCreator().find(name);
	assert(it != UserFilterCreator().end());
	auto filter =  it->second();
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

BOOST_CLASS_EXPORT(SerializableDataImpl<float>)
