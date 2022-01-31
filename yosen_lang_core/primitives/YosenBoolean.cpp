#include "YosenBoolean.h"
#include <YosenEnvironment.h>

namespace yosen
{
	YosenBoolean::YosenBoolean()
	{
		register_runtime_operator_functions();
	}

	YosenBoolean::YosenBoolean(bool val) : value(val)
	{
		register_runtime_operator_functions();
	}
	
	YosenObject* YosenBoolean::clone()
	{
		auto new_obj = allocate_object<YosenBoolean>(this->value);
		return new_obj;
	}
	
	std::string YosenBoolean::to_string()
	{
		return (value) ? "true" : "false";
	}
	
	const char* YosenBoolean::runtime_name() const
	{
		return "Boolean";
	}

	void YosenBoolean::register_runtime_operator_functions()
	{
		add_runtime_operator_function(RuntimeOperator::BoolOpEqu,			MEMBER_FUNCTION(operator_equ));
		add_runtime_operator_function(RuntimeOperator::BoolOpNotEqu,		MEMBER_FUNCTION(operator_notequ));
		add_runtime_operator_function(RuntimeOperator::BoolOpGreaterThan,	MEMBER_FUNCTION(operator_greater));
		add_runtime_operator_function(RuntimeOperator::BoolOpLessThan,		MEMBER_FUNCTION(operator_less));
		add_runtime_operator_function(RuntimeOperator::BoolOpOr,			MEMBER_FUNCTION(operator_or));
		add_runtime_operator_function(RuntimeOperator::BoolOpAnd,			MEMBER_FUNCTION(operator_and));
	}

	YosenObject* YosenBoolean::operator_equ(YosenObject* lhs, YosenObject* rhs)
	{
		if (strcmp(rhs->runtime_name(), "Boolean") != 0)
		{
			auto ex_reason = std::string("cannot call a boolean operator on types Boolean and ") + rhs->runtime_name();
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		auto left_val = static_cast<YosenBoolean*>(lhs)->value;
		auto right_val = static_cast<YosenBoolean*>(rhs)->value;;

		return allocate_object<YosenBoolean>(left_val == right_val);
	}

	YosenObject* YosenBoolean::operator_notequ(YosenObject* lhs, YosenObject* rhs)
	{
		if (strcmp(rhs->runtime_name(), "Boolean") != 0)
		{
			auto ex_reason = std::string("cannot call a boolean operator on types Boolean and ") + rhs->runtime_name();
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		auto left_val = static_cast<YosenBoolean*>(lhs)->value;
		auto right_val = static_cast<YosenBoolean*>(rhs)->value;;

		return allocate_object<YosenBoolean>(left_val != right_val);
	}

	YosenObject* YosenBoolean::operator_greater(YosenObject* lhs, YosenObject* rhs)
	{
		if (strcmp(rhs->runtime_name(), "Boolean") != 0)
		{
			auto ex_reason = std::string("cannot call a boolean operator on types Boolean and ") + rhs->runtime_name();
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		auto left_val = static_cast<YosenBoolean*>(lhs)->value;
		auto right_val = static_cast<YosenBoolean*>(rhs)->value;;

		return allocate_object<YosenBoolean>(left_val > right_val);
	}

	YosenObject* YosenBoolean::operator_less(YosenObject* lhs, YosenObject* rhs)
	{
		if (strcmp(rhs->runtime_name(), "Boolean") != 0)
		{
			auto ex_reason = std::string("cannot call a boolean operator on types Boolean and ") + rhs->runtime_name();
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		auto left_val = static_cast<YosenBoolean*>(lhs)->value;
		auto right_val = static_cast<YosenBoolean*>(rhs)->value;;

		return allocate_object<YosenBoolean>(left_val < right_val);
	}

	YosenObject* YosenBoolean::operator_or(YosenObject* lhs, YosenObject* rhs)
	{
		if (strcmp(rhs->runtime_name(), "Boolean") != 0)
		{
			auto ex_reason = std::string("cannot call a boolean operator on types Boolean and ") + rhs->runtime_name();
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		auto left_val = static_cast<YosenBoolean*>(lhs)->value;
		auto right_val = static_cast<YosenBoolean*>(rhs)->value;;

		return allocate_object<YosenBoolean>(left_val || right_val);
	}

	YosenObject* YosenBoolean::operator_and(YosenObject* lhs, YosenObject* rhs)
	{
		if (strcmp(rhs->runtime_name(), "Boolean") != 0)
		{
			auto ex_reason = std::string("cannot call a boolean operator on types Boolean and ") + rhs->runtime_name();
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		auto left_val = static_cast<YosenBoolean*>(lhs)->value;
		auto right_val = static_cast<YosenBoolean*>(rhs)->value;;

		return allocate_object<YosenBoolean>(left_val && right_val);
	}
}
