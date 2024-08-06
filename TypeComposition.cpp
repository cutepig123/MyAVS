#include "TypeComposition.h"
#include <map>

std::map<std::string, std::set<Field> >&	GetTypeRepo()
{
	static std::map<std::string, std::set<Field> > repo;
	return repo;
}

int RegisterType(const char* name, std::set<Field> const& children)
{
	GetTypeRepo()[name] = children;
	return 0;
}

const std::set<Field>& GetTypeItems(const char* name)
{
	return GetTypeRepo().at(name);
}

static int g_typeInt = RegisterType("int", std::set<Field>{});
static int g_typeCoord = RegisterType("Coord", std::set<Field>{Field{ "int","x" }, Field{ "int","y" }});
static int g_typeSize = RegisterType("Size", std::set<Field>{Field{ "int","x" }, Field{ "int","y" }});
static int g_typeWindow = RegisterType("Window", std::set<Field>{Field{ "Coord", "offset" }, Field{ "Size", "size" }});
