#include "stdafx.h"
#include "TypeComposition.h"
#include <nlohmann/json.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <map>

using json = nlohmann::json;

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

std::shared_ptr<MyObject> NewObject(const char* type)
{
	auto ret = std::make_shared<MyObject>();
	ret->type = type;
	
	const auto& fields = GetTypeItems(type);
	if(fields.empty())
		ret->value = std::string();
	else
	{
		ret->value = MyObject::Struct();

		for(const auto& field: fields)
		{
			boost::get<MyObject::Struct>(ret->value).fields[field.name] = NewObject(field.type.c_str());
		}
	}
	return ret;
}

namespace nlohmann {
    
	class to_json_visitor
		: public boost::static_visitor<>
	{
		json& t_;
	public:
		to_json_visitor(json& t) :t_(t) {}

		template <class T>
		void operator()(T const & i) const
		{
			t_ = i;
		}

	};

	class from_json_visitor
		: public boost::static_visitor<>
	{
		const json& t_;
	public:
		from_json_visitor(const json& t) :t_(t) {}

		template <class T>
		void operator()(T & i) const
		{
			i = t_;
		}

	};


	template <>
	struct adl_serializer<boost::variant<std::string, double, MyObject::Struct>> {
		static void to_json(json& j, const boost::variant<std::string, double, MyObject::Struct>& opt) {
			boost::apply_visitor(to_json_visitor(j), opt);
		}

		static void from_json(const json& j, boost::variant<std::string, double, MyObject::Struct>& opt) {
			boost::apply_visitor(from_json_visitor(j), opt);
		}
	};

	template <>
    struct adl_serializer<std::shared_ptr<MyObject>> {
        static void to_json(json& j, const std::shared_ptr<MyObject>& opt) {
            if (!opt) {
                j = nullptr;
            } else {
				j["type"] = opt->type;
				j["value"] = opt->value;
            }
        }

        static void from_json(const json& j, std::shared_ptr<MyObject>& opt) {
            if (j.is_null()) {
                opt.reset();
            } else {
				opt.reset(new MyObject);
				opt->type = j["type"].template get<std::string>();
				json jvalue = j["value"];
				decltype(opt->value) value = jvalue.get<decltype(opt->value)>();
				opt->value = value;
            }
        }
    };

	template <>
	struct adl_serializer<MyObject::Struct> {
		static void to_json(json& j, const MyObject::Struct& opt) {
				j = opt.fields;
		}

		static void from_json(const json& j, MyObject::Struct& opt) {
			if (j.is_null()) {
				opt.fields.clear();
			}
			else {
				opt.fields = j.template get<decltype(opt.fields)>();;
			}
		}
	};

	
}

std::shared_ptr<MyObject>	ObjectFromJSONString(const char* s)
{
	json data = json::parse(s);
	std::shared_ptr<MyObject> o = data;
	return o;
}

std::string ObjectToJSONString(std::shared_ptr<MyObject> const& o)
{
	json j = o;
	return j.dump();
}

static int g_typeInt = RegisterType("int", std::set<Field>{});
static int g_typeStr = RegisterType("string", std::set<Field>{});
static int g_typeCoord = RegisterType("Coord", std::set<Field>{Field{ "int","x" }, Field{ "int","y" }});
static int g_typeSize = RegisterType("Size", std::set<Field>{Field{ "int","x" }, Field{ "int","y" }});
static int g_typeWindow = RegisterType("Window", std::set<Field>{Field{ "Coord", "offset" }, Field{ "Size", "size" }});


std::shared_ptr<MyObject> NewStringObject(const char* value)
{
	auto o = NewObject("string");
	o->value = value;
	return o;
}

std::string CheckStringObject(std::shared_ptr<MyObject> const& o)
{
	assert(o->type == "string");
	return boost::get<std::string>(o->value);
}

std::shared_ptr<MyObject> NewIntObject(const char* value)
{
	auto o = NewObject("int");
	o->value = std::string(value);
	return o;
}

std::string CheckIntObject(std::shared_ptr<MyObject> const& o)
{
	assert(o->type == "int");
	return boost::get<std::string>(o->value);
}

void TestJSON()
{
	{
		auto o = NewObject("Window");
		auto s = ObjectToJSONString(o);
		TRACE("%s\n", s.c_str());
	}

	{
		auto o = NewObject("int");
		auto s = ObjectToJSONString(o);
		TRACE("%s\n", s.c_str());
	}

	{
		auto o = NewObject("string");
		auto s = ObjectToJSONString(o);
		TRACE("%s\n", s.c_str());
	}
}
