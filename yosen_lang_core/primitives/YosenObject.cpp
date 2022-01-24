#include "YosenObject.h"
#include <sstream>
#include <algorithm>
#include <stdarg.h>

// Primitive Types
#include "YosenTuple.h"
#include "YosenInteger.h"
#include "YosenString.h"

namespace yosen
{
	YosenObject* YosenObject_Null = nullptr;
	static uint64_t s_total_allocated_objects = 0;

	YosenObject::YosenObject()
	{
		const void* this_address = static_cast<const void*>(this);
		std::stringstream ss;
		ss << std::hex << this_address;

		m_string_repr = std::string("<YosenObject at 0x") + ss.str() + ">";
	}

	YosenObject* YosenObject::clone()
	{
		YosenObject* new_obj = allocate_object<YosenObject>();
		if (this->m_string_repr._Equal("null"))
			new_obj->m_string_repr = "null";

		return new_obj;
	}

	std::string YosenObject::to_string()
	{
		return m_string_repr;
	}

	const char* YosenObject::runtime_name() const
	{
		return "Object";
	}

	void YosenObject::add_member_native_function(const std::string& name, ys_member_native_fn_t fn)
	{
		m_member_native_functions[name] = fn;
	}

	bool YosenObject::has_member_native_function(const std::string& name)
	{
		return m_member_native_functions.find(name) != m_member_native_functions.end();
	}

	YosenObject* YosenObject::call_member_native_function(const std::string& name, YosenObject* args)
	{
		if (!has_member_native_function(name))
		{
			printf("No member function '%s' found for object of type %s\n", name.c_str(), this->runtime_name());
			return YosenObject_Null->clone();
		}

		auto fn = m_member_native_functions[name];
		return fn(this, args);
	}

	void __yosen_register_allocated_object(void* obj)
	{
		++s_total_allocated_objects;
	}

	uint64_t __yosen_get_total_allocated_objects()
	{
		return s_total_allocated_objects;
	}

	void free_object(YosenObject* obj)
	{
		delete obj;
		--s_total_allocated_objects;

#ifdef PROFILE_OBJECT_ALLOCATION
		printf("YosenObject Freed,      total object count: %zi\n", s_total_allocated_objects);
#endif // PROFILE_OBJECT_ALLOCATION
	}
	
	bool arg_parse(YosenObject* obj, const char* fmt, ...)
	{
		if (strcmp(obj->runtime_name(), "Tuple") != 0)
		{
			throw "Argument not a tuple";
		}

		YosenTuple* args_tuple = dynamic_cast<YosenTuple*>(obj);
		
		size_t fmt_arg_count = std::min(strlen(fmt), args_tuple->items.size());

		va_list args;
		va_start(args, fmt);

		for (size_t i = 0; i < fmt_arg_count; i++)
		{
			char arg_type = fmt[i];
			switch (arg_type)
			{
			case 'i': {
				uint64_t* p_arg = va_arg(args, uint64_t*);
				YosenInteger* t_arg = reinterpret_cast<YosenInteger*>(args_tuple->items[i]);
				if (strcmp(t_arg->runtime_name(), "Int") != 0)
				{
					throw "Not an Integer";
					return false;
				}

				*p_arg = t_arg->value;
				break;
			}
			case 's': {
				char** p_arg = va_arg(args, char**);
				YosenString* t_arg = reinterpret_cast<YosenString*>(args_tuple->items[i]);
				if (strcmp(t_arg->runtime_name(), "String") != 0)
				{
					throw "Not a String";
					return false;
				}

				*p_arg = (char*)t_arg->value.c_str();
				break;
			}
			case 'o': {
				YosenObject** p_arg = va_arg(args, YosenObject**);
				YosenObject* t_arg = args_tuple->items[i];
				*p_arg = t_arg;
				break;
			}
			default: {
				int _ = va_arg(args, int);
				printf("YosenArg_Parse::Warn: Unknown format argument '%c'\n", fmt[i]);
				break;
			}
			}
		}

		va_end(args);
		return true;
	}
}
