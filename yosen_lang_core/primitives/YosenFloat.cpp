#include "YosenFloat.h"
#include <YosenEnvironment.h>
#include <sstream>

namespace yosen
{
	YosenFloat::YosenFloat()
	{
		register_runtime_operator_functions();
	}

	YosenFloat::YosenFloat(double val) : value(val)
	{
		register_runtime_operator_functions();
	}

	YosenObject* YosenFloat::clone()
	{
		auto new_obj = allocate_object<YosenFloat>(this->value);
		return new_obj;
	}

	std::string YosenFloat::to_string()
	{
		std::stringstream ss;
		ss << this->value;

		return ss.str();
	}

	const char* YosenFloat::runtime_name() const
	{
		return "Float";
	}

	void YosenFloat::register_runtime_operator_functions()
	{
		add_runtime_operator_function(RuntimeOperator::BinOpAdd, MEMBER_FUNCTION(operator_add));
		add_runtime_operator_function(RuntimeOperator::BinOpSub, MEMBER_FUNCTION(operator_sub));
		add_runtime_operator_function(RuntimeOperator::BinOpMul, MEMBER_FUNCTION(operator_mul));
		add_runtime_operator_function(RuntimeOperator::BinOpDiv, MEMBER_FUNCTION(operator_div));
		add_runtime_operator_function(RuntimeOperator::BoolOpEqu, MEMBER_FUNCTION(operator_equ));
		add_runtime_operator_function(RuntimeOperator::BoolOpNotEqu, MEMBER_FUNCTION(operator_notequ));
		add_runtime_operator_function(RuntimeOperator::BoolOpGreaterThan, MEMBER_FUNCTION(operator_greater));
		add_runtime_operator_function(RuntimeOperator::BoolOpLessThan, MEMBER_FUNCTION(operator_less));
	}

	YosenObject* YosenFloat::operator_add(YosenObject* lhs, YosenObject* rhs)
	{
		auto left_val = static_cast<YosenFloat*>(lhs)->value;
		auto right_val = static_cast<YosenFloat*>(rhs)->value;

		return allocate_object<YosenFloat>(left_val + right_val);
	}

	YosenObject* YosenFloat::operator_sub(YosenObject* lhs, YosenObject* rhs)
	{
		auto left_val = static_cast<YosenFloat*>(lhs)->value;
		auto right_val = static_cast<YosenFloat*>(rhs)->value;

		return allocate_object<YosenFloat>(left_val - right_val);
	}

	YosenObject* YosenFloat::operator_mul(YosenObject* lhs, YosenObject* rhs)
	{
		auto left_val = static_cast<YosenFloat*>(lhs)->value;
		auto right_val = static_cast<YosenFloat*>(rhs)->value;

		return allocate_object<YosenFloat>(left_val * right_val);
	}

	YosenObject* YosenFloat::operator_div(YosenObject* lhs, YosenObject* rhs)
	{
		auto left_val = static_cast<YosenFloat*>(lhs)->value;
		auto right_val = static_cast<YosenFloat*>(rhs)->value;

		return allocate_object<YosenFloat>(left_val / right_val);
	}

	YosenObject* YosenFloat::operator_equ(YosenObject* lhs, YosenObject* rhs)
	{
		if (strcmp(rhs->runtime_name(), "Float") != 0)
		{
			auto ex_reason = std::string("cannot compare objects of type ") + rhs->runtime_name() + " and Float";
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		auto left_val = static_cast<YosenFloat*>(lhs)->value;
		auto right_val = static_cast<YosenFloat*>(rhs)->value;

		return allocate_object<YosenBoolean>(left_val == right_val);
	}

	YosenObject* YosenFloat::operator_notequ(YosenObject* lhs, YosenObject* rhs)
	{
		if (strcmp(rhs->runtime_name(), "Float") != 0)
		{
			auto ex_reason = std::string("cannot compare objects of type ") + rhs->runtime_name() + " and Float";
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		auto left_val = static_cast<YosenFloat*>(lhs)->value;
		auto right_val = static_cast<YosenFloat*>(rhs)->value;

		return allocate_object<YosenBoolean>(left_val != right_val);
	}

	YosenObject* YosenFloat::operator_greater(YosenObject* lhs, YosenObject* rhs)
	{
		if (strcmp(rhs->runtime_name(), "Float") != 0)
		{
			auto ex_reason = std::string("cannot compare objects of type ") + rhs->runtime_name() + " and Float";
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		auto left_val = static_cast<YosenFloat*>(lhs)->value;
		auto right_val = static_cast<YosenFloat*>(rhs)->value;

		return allocate_object<YosenBoolean>(left_val > right_val);
	}

	YosenObject* YosenFloat::operator_less(YosenObject* lhs, YosenObject* rhs)
	{
		if (strcmp(rhs->runtime_name(), "Float") != 0)
		{
			auto ex_reason = std::string("cannot compare objects of type ") + rhs->runtime_name() + " and Float";
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		auto left_val = static_cast<YosenFloat*>(lhs)->value;
		auto right_val = static_cast<YosenFloat*>(rhs)->value;

		return allocate_object<YosenBoolean>(left_val < right_val);
	}
}
