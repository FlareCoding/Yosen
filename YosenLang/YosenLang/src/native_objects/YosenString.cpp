#include "YosenString.h"

namespace yosen
{
	YosenString::YosenString()
	{
		register_member_functions();
	}

	YosenString::YosenString(const std::string& val)
		: value(val)
	{
		register_member_functions();
	}
	
	YosenObject* YosenString::clone()
	{
		auto new_obj = allocate_object<YosenString>(this->value);
		return new_obj;
	}

	std::string YosenString::to_string()
	{
		return this->value;
	}

	const char* YosenString::runtime_name() const
	{
		return "String";
	}

	void YosenString::register_member_functions()
	{
		add_member_function("reverse", MEMBER_FUNCTION(reverse));
	}

	YosenObject* YosenString::reverse(YosenObject* args)
	{
		std::string val_copy(value);
		std::reverse(val_copy.begin(), val_copy.end());

		return allocate_object<YosenString>(val_copy);
	}
}
