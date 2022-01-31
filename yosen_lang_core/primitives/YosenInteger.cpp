#include "YosenInteger.h"
#include <YosenEnvironment.h>

namespace yosen
{
	YosenInteger::YosenInteger()
	{
		register_runtime_operator_functions();
	}

	YosenInteger::YosenInteger(int64_t val) : value(val)
	{
		register_runtime_operator_functions();
	}

	YosenObject* YosenInteger::clone()
	{
		auto new_obj = allocate_object<YosenInteger>(this->value);
		return new_obj;
	}

	std::string YosenInteger::to_string()
	{
		return std::to_string(value);
	}
	
	const char* YosenInteger::runtime_name() const
	{
		return "Integer";
	}

	void YosenInteger::register_runtime_operator_functions()
	{
		add_runtime_operator_function(RuntimeOperator::BinOpAdd,			MEMBER_FUNCTION(operator_add));
		add_runtime_operator_function(RuntimeOperator::BinOpSub,			MEMBER_FUNCTION(operator_sub));
		add_runtime_operator_function(RuntimeOperator::BinOpMul,			MEMBER_FUNCTION(operator_mul));
		add_runtime_operator_function(RuntimeOperator::BinOpDiv,			MEMBER_FUNCTION(operator_div));
		add_runtime_operator_function(RuntimeOperator::BinOpMod,			MEMBER_FUNCTION(operator_mod));
		add_runtime_operator_function(RuntimeOperator::BoolOpEqu,			MEMBER_FUNCTION(operator_equ));
		add_runtime_operator_function(RuntimeOperator::BoolOpNotEqu,		MEMBER_FUNCTION(operator_notequ));
		add_runtime_operator_function(RuntimeOperator::BoolOpGreaterThan,	MEMBER_FUNCTION(operator_greater));
		add_runtime_operator_function(RuntimeOperator::BoolOpLessThan,		MEMBER_FUNCTION(operator_less));
	}

	YosenObject* YosenInteger::operator_add(YosenObject* lhs, YosenObject* rhs)
	{
		auto left_val = static_cast<YosenInteger*>(lhs)->value;
		auto right_val = static_cast<YosenInteger*>(rhs)->value;

		return allocate_object<YosenInteger>(left_val + right_val);
	}

	YosenObject* YosenInteger::operator_sub(YosenObject* lhs, YosenObject* rhs)
	{
		auto left_val = static_cast<YosenInteger*>(lhs)->value;
		auto right_val = static_cast<YosenInteger*>(rhs)->value;

		return allocate_object<YosenInteger>(left_val - right_val);
	}

	YosenObject* YosenInteger::operator_mul(YosenObject* lhs, YosenObject* rhs)
	{
		auto left_val = static_cast<YosenInteger*>(lhs)->value;
		auto right_val = static_cast<YosenInteger*>(rhs)->value;

		return allocate_object<YosenInteger>(left_val * right_val);
	}

	YosenObject* YosenInteger::operator_div(YosenObject* lhs, YosenObject* rhs)
	{
		auto left_val = static_cast<YosenInteger*>(lhs)->value;
		auto right_val = static_cast<YosenInteger*>(rhs)->value;

		return allocate_object<YosenInteger>(left_val / right_val);
	}
	
	YosenObject* YosenInteger::operator_mod(YosenObject* lhs, YosenObject* rhs)
	{
		auto left_val = static_cast<YosenInteger*>(lhs)->value;
		auto right_val = static_cast<YosenInteger*>(rhs)->value;

		return allocate_object<YosenInteger>(left_val % right_val);
	}

	YosenObject* YosenInteger::operator_equ(YosenObject* lhs, YosenObject* rhs)
	{
		if (strcmp(rhs->runtime_name(), "Integer") != 0)
		{
			auto ex_reason = std::string("cannot compare objects of type ") + rhs->runtime_name() + " and Integer";
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		auto left_val = static_cast<YosenInteger*>(lhs)->value;
		auto right_val = static_cast<YosenInteger*>(rhs)->value;

		return allocate_object<YosenBoolean>(left_val == right_val);
	}
	
	YosenObject* YosenInteger::operator_notequ(YosenObject* lhs, YosenObject* rhs)
	{
		if (strcmp(rhs->runtime_name(), "Integer") != 0)
		{
			auto ex_reason = std::string("cannot compare objects of type ") + rhs->runtime_name() + " and Integer";
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		auto left_val = static_cast<YosenInteger*>(lhs)->value;
		auto right_val = static_cast<YosenInteger*>(rhs)->value;

		return allocate_object<YosenBoolean>(left_val != right_val);
	}
	
	YosenObject* YosenInteger::operator_greater(YosenObject* lhs, YosenObject* rhs)
	{
		if (strcmp(rhs->runtime_name(), "Integer") != 0)
		{
			auto ex_reason = std::string("cannot compare objects of type ") + rhs->runtime_name() + " and Integer";
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		auto left_val = static_cast<YosenInteger*>(lhs)->value;
		auto right_val = static_cast<YosenInteger*>(rhs)->value;

		return allocate_object<YosenBoolean>(left_val > right_val);
	}
	
	YosenObject* YosenInteger::operator_less(YosenObject* lhs, YosenObject* rhs)
	{
		if (strcmp(rhs->runtime_name(), "Integer") != 0)
		{
			auto ex_reason = std::string("cannot compare objects of type ") + rhs->runtime_name() + " and Integer";
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		auto left_val = static_cast<YosenInteger*>(lhs)->value;
		auto right_val = static_cast<YosenInteger*>(rhs)->value;

		return allocate_object<YosenBoolean>(left_val < right_val);
	}
}
