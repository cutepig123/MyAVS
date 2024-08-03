#pragma once
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost\serialization\nvp.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <typeinfo>

class SerializableData
{
	friend class boost::serialization::access;
	template <class Archive> void serialize(Archive & ar, unsigned)
	{
	}  
public:
	virtual ~SerializableData()=0{}
	virtual const void* cast(const std::type_info&) const = 0;
	
	template <class T>
	const T* TryCast() const
	{
		return (T*)cast(typeid(T));
	}

	template <class T>
	const T& Cast() const
	{
		return *TryCast<T>();
	}

	// For cstruct, name: field name support child
	// If name is empty, return this
	virtual void SetItem(const char* name, const SerializableData&) = 0;
	virtual std::string GetItem(const char* name) = 0;

	void SetItem(const char* name, const char* value);

	virtual std::string GetDisplayText() const = 0;
	virtual void SetDisplayText(const char* text) = 0;
	virtual std::vector<std::string> GetAllSubItems() const = 0;
};

template <class T>
std::string ToStringT(T const& t)
{
	std::stringstream ss;
	boost::archive::xml_oarchive ia(ss);
	ia << boost::serialization::make_nvp("value", t);
	return ss.str();
}

template <class T>
T FromStringT(const char* s)
{
	T p;
	std::stringstream ss;
	ss << s;
	boost::archive::xml_iarchive ia(ss);
	ia >> boost::serialization::make_nvp("value", p);
	return p;
}

template <class T>
std::string ToString(T const& t)
{
	boost::shared_ptr<SerializableData> p(new SerializableDataImpl<T>(t));
	return ToStringT(p);
}

inline boost::shared_ptr<SerializableData> FromString(const char* s)
{
	return FromStringT< boost::shared_ptr<SerializableData>>(s);
}

inline void SerializableData::SetItem(const char* name, const char* value)
{
	SetItem(name, *FromString(value));
}

template <class T>
class SerializableDataImpl:public SerializableData
{
	T t_;
	
	friend class boost::serialization::access;
	template <class Archive> void serialize(Archive & ar, unsigned){
		using namespace boost::serialization;
		//void_cast_register<SerializableDataImpl<T>,SerializableData>();
		ar & make_nvp("base", base_object<SerializableData>(*this));
		ar & BOOST_SERIALIZATION_NVP(t_);
	}  
public:
	SerializableDataImpl(){}
	explicit SerializableDataImpl(T const& t):t_(t){}
	virtual const void* cast(const std::type_info& ti) const override
	{
		if(ti!=typeid(T))
			return nullptr;
		return &t_;
	}

	virtual void SetItem(const char* name, SerializableData const& value) override
	{
		assert(!name || !name[0]);
		t_ = value.Cast<T>();
	}
	virtual std::string GetItem(const char* name) override
	{
		assert(!name || !name[0]);
		return ToString(t_);
	}

	virtual std::string GetDisplayText() const override
	{
		std::stringstream ss;
		ss << t_;
		return ss.str();
	}
	virtual void SetDisplayText(const char* text) override
	{
		std::stringstream ss;
		ss << text;
		ss >> t_;
		assert(!ss.fail());
	}
	virtual std::vector<std::string> GetAllSubItems() const override
	{
		return std::vector<std::string>();
	}
};

class UserFilter
{
	std::string name_;
	/*
	There are 2 type data, one is the "graph" data, one is the runtime data
	the graph describes filters connections, or initial values
	the runtime data contaisn the current value of the port
	the graph is managed by MainFilter
	the runtime data is stored here
	*/
	std::map<std::string, boost::shared_ptr<SerializableData>> ins_;
	std::map<std::string, boost::shared_ptr<SerializableData>> outs_;

	friend struct Access;
public:
	virtual ~UserFilter() = 0 {};

	virtual void Define() = 0;
	virtual void Invoke() = 0;

	// Detail
	const char* Name() { return name_.c_str(); }


protected:
	// For derived class to call
	void SetName(const char* name) { name_ = name; }

	template <class T>
	void AddInput(const char* name, const T& default_value)
	{
		assert(strstr(name, ".")==0);
		ins_[name].reset(new SerializableDataImpl<T>(default_value));
	}
	template <class T>
	void AddOutput(const char* name, const T& default_value)
	{
		assert(strstr(name, ".")==0);
		outs_[name].reset(new SerializableDataImpl<T>(default_value));
	}

	template <class T>
	const T* TryReadInput(const char* name)
	{
		return ins_.at(name)->TryCast<T>();
	}

	template <class T>
	T ReadInput(const char* name)
	{
		return *TryReadInput<T>(name);
	}

	template <class T>
	void WriteOutput(const char* name, T const& t)
	{
		outs_.at(name).reset(new SerializableDataImpl<T>(t));
	}
private:

};

int RegisterFilter(boost::function<UserFilter*()> const& create);
std::unique_ptr<UserFilter> CreateFilter(const char* name);
std::vector<std::string> GetAllUserFilters();

#define	AVS_REGSITER_STRUCT(T)
