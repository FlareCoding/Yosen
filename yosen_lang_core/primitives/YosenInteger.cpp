#include "YosenInteger.h"

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
		add_runtime_operator_function(RuntimeOperator::BinOpAdd, MEMBER_FUNCTION(operator_add));
		add_runtime_operator_function(RuntimeOperator::BinOpSub, MEMBER_FUNCTION(operator_sub));
		add_runtime_operator_function(RuntimeOperator::BinOpMul, MEMBER_FUNCTION(operator_mul));
		add_runtime_operator_function(RuntimeOperator::BinOpDiv, MEMBER_FUNCTION(operator_div));
		add_runtime_operator_function(RuntimeOperator::BinOpMod, MEMBER_FUNCTION(operator_mod));
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
}
