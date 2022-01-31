#pragma once
#include "YosenObject.h"

namespace yosen
{
	class YosenInteger : public YosenObject
	{
	public:
		YOSENAPI YosenInteger();
		YOSENAPI YosenInteger(int64_t val);

		YOSENAPI YosenObject* clone() override;
		YOSENAPI std::string to_string() override;
		YOSENAPI const char* runtime_name() const override;

		int64_t value = 0;

	private:
		void register_runtime_operator_functions();

		YosenObject* operator_add(YosenObject* lhs, YosenObject* rhs);
		YosenObject* operator_sub(YosenObject* lhs, YosenObject* rhs);
		YosenObject* operator_mul(YosenObject* lhs, YosenObject* rhs);
		YosenObject* operator_div(YosenObject* lhs, YosenObject* rhs);
		YosenObject* operator_mod(YosenObject* lhs, YosenObject* rhs);
	};
}
