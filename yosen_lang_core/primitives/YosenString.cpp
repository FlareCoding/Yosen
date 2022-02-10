#include "YosenString.h"
#include <YosenEnvironment.h>

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
		add_member_native_function("append", MEMBER_FUNCTION(append));
	}

	YosenObject* YosenString::reverse(YosenObject* self, YosenObject* args)
	{
		auto this_obj = static_cast<YosenString*>(self);

		std::string val_copy(this_obj->value);
		std::reverse(val_copy.begin(), val_copy.end());

		return allocate_object<YosenString>(val_copy);
	}

	YosenObject* YosenString::append(YosenObject* self, YosenObject* args)
	{
		YosenObject* rhs;
		arg_parse(args, "o", &rhs);

		if (strcmp(rhs->runtime_name(), "String") != 0)
		{
			auto ex_reason = std::string("cannot append object of type ") + rhs->runtime_name() + " to a string";
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		auto this_obj = static_cast<YosenString*>(self);
		auto rhs_obj = static_cast<YosenString*>(rhs);

		this_obj->value.append(rhs_obj->value);
		return YosenObject_Null->clone();
	}
	
	void YosenString::register_runtime_operator_functions()
	{
		add_runtime_operator_function(RuntimeOperator::BinOpAdd,		MEMBER_FUNCTION(operator_add));
		add_runtime_operator_function(RuntimeOperator::BoolOpEqu,		MEMBER_FUNCTION(operator_equ));
		add_runtime_operator_function(RuntimeOperator::BoolOpNotEqu,	MEMBER_FUNCTION(operator_notequ));
	}

	YosenObject* YosenString::operator_add(YosenObject* lhs, YosenObject* rhs)
	{
		auto left_string  = static_cast<YosenString*>(lhs)->value;
		auto right_string = rhs->to_string();

		auto result_string = left_string + right_string;

		return allocate_object<YosenString>(result_string);
	}

	YosenObject* YosenString::operator_equ(YosenObject* lhs, YosenObject* rhs)
	{
		if (strcmp(rhs->runtime_name(), "String") != 0)
		{
			auto ex_reason = std::string("cannot compare objects of type ") + rhs->runtime_name() + " and String";
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		auto left_val = static_cast<YosenString*>(lhs)->value;
		auto right_val = static_cast<YosenString*>(rhs)->value;

		return allocate_object<YosenBoolean>(left_val == right_val);
	}
	
	YosenObject* YosenString::operator_notequ(YosenObject* lhs, YosenObject* rhs)
	{
		if (strcmp(rhs->runtime_name(), "String") != 0)
		{
			auto ex_reason = std::string("cannot compare objects of type ") + rhs->runtime_name() + " and String";
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		auto left_val = static_cast<YosenString*>(lhs)->value;
		auto right_val = static_cast<YosenString*>(rhs)->value;

		return allocate_object<YosenBoolean>(left_val != right_val);
	}
}
