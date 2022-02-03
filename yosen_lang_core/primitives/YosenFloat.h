#pragma once
#include "YosenObject.h"

namespace yosen
{
	class YosenFloat : public YosenObject
	{
	public:
		YOSENAPI YosenFloat();
		YOSENAPI YosenFloat(double val);

		YOSENAPI YosenObject* clone() override;
		YOSENAPI std::string to_string() override;
		YOSENAPI const char* runtime_name() const override;

		double value = 0;

	private:
		void register_runtime_operator_functions();

		YosenObject* operator_add(YosenObject* lhs, YosenObject* rhs);
		YosenObject* operator_sub(YosenObject* lhs, YosenObject* rhs);
		YosenObject* operator_mul(YosenObject* lhs, YosenObject* rhs);
		YosenObject* operator_div(YosenObject* lhs, YosenObject* rhs);
		YosenObject* operator_equ(YosenObject* lhs, YosenObject* rhs);
		YosenObject* operator_notequ(YosenObject* lhs, YosenObject* rhs);
		YosenObject* operator_greater(YosenObject* lhs, YosenObject* rhs);
		YosenObject* operator_less(YosenObject* lhs, YosenObject* rhs);
	};
}
