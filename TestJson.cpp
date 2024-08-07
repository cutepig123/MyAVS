#include <nlohmann/json.hpp>
#include <boost/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <map>

using json = nlohmann::json;

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
	struct adl_serializer<boost::variant<std::string, double>> {
		static void to_json(json& j, const boost::variant<std::string, double>& opt) {
			boost::apply_visitor(to_json_visitor(j), opt);
		}

		static void from_json(const json& j, boost::variant<std::string, double>& opt) {
			boost::apply_visitor(from_json_visitor(j), opt);
		}
	};

}

void test()
{
	boost::variant<std::string, double> a;
	json b = a;
	a = b.template get<boost::variant<std::string, double>>();
}