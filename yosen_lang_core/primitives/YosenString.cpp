#include "YosenString.h"

namespace yosen
{
	YosenString::YosenString()
	{
		register_member_native_functions();
		register_runtime_operator_functions();
	}

	YosenString::YosenString(const std::string& val)
		: value(val)
	{
		register_member_native_functions();
		register_runtime_operator_functions();
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

	void YosenString::register_member_native_functions()
	{
		add_member_native_function("reverse", MEMBER_FUNCTION(reverse));
	}

	YosenObject* YosenString::reverse(YosenObject* self, YosenObject* args)
	{
		auto this_obj = static_cast<YosenString*>(self);

		std::string val_copy(this_obj->value);
		std::reverse(val_copy.begin(), val_copy.end());

		return allocate_object<YosenString>(val_copy);
	}
	
	void YosenString::register_runtime_operator_functions()
	{
		add_runtime_operator_function(RuntimeOperator::BinOpAdd, MEMBER_FUNCTION(operator_add));
	}

	YosenObject* YosenString::operator_add(YosenObject* lhs, YosenObject* rhs)
	{
		auto left_string  = static_cast<YosenString*>(lhs)->value;
		auto right_string = rhs->to_string();

		auto result_string = left_string + right_string;

		return allocate_object<YosenString>(result_string);
	}
}
