#include "YosenReference.h"

namespace yosen
{
	YosenReference::YosenReference()
	{
		register_member_native_functions();
	}

	YosenReference::YosenReference(YosenObject* obj)
		: obj(obj)
	{
		register_member_native_functions();
	}

	YosenReference::~YosenReference()
	{
	}

	YosenObject* YosenReference::clone()
	{
		return allocate_object<YosenReference>(obj);
	}

	std::string YosenReference::to_string()
	{
		return "Ref" + obj->instance_info();
	}

	const char* YosenReference::runtime_name() const
	{
		return "Ref";
	}

	void YosenReference::register_member_native_functions()
	{
		add_member_native_function("obj", MEMBER_FUNCTION(get_obj));
	}

	YosenObject* YosenReference::get_obj(YosenObject* self, YosenObject* args)
	{
		auto this_obj = static_cast<YosenReference*>(self);
		return this_obj->obj->clone();
	}

	bool YosenReference::has_member_native_function(const std::string& name)
	{
		if (m_member_native_functions.find(name) != m_member_native_functions.end())
			return true;

		return obj->has_member_native_function(name);
	}

	YosenObject* YosenReference::call_member_native_function(const std::string& name, YosenObject* args)
	{
		if (has_member_native_function(name))
		{
			auto fn = m_member_native_functions[name];
			return fn(this, args);
		}

		return obj->call_member_native_function(name, args);
	}
	
	bool YosenReference::has_member_runtime_function(const std::string& name)
	{
		return obj->has_member_runtime_function(name);
	}
	
	ys_runtime_function_t YosenReference::get_member_runtime_function(const std::string& name)
	{
		return obj->get_member_runtime_function(name);
	}
	
	bool YosenReference::has_member_variable(const std::string& name)
	{
		return obj->has_member_variable(name);
	}
	
	YosenObject* YosenReference::get_member_variable(const std::string& name)
	{
		return obj->get_member_variable(name);
	}
	
	void YosenReference::set_member_variable(const std::string& name, YosenObject* value)
	{
		obj->set_member_variable(name, value);
	}
}
