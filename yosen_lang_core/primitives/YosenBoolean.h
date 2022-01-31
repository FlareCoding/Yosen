#pragma once
#include "YosenObject.h"

namespace yosen
{
	class YosenBoolean : public YosenObject
	{
	public:
		YOSENAPI YosenBoolean();
		YOSENAPI YosenBoolean(bool val);

		YOSENAPI YosenObject* clone() override;
		YOSENAPI std::string to_string() override;
		YOSENAPI const char* runtime_name() const override;

		bool value = false;

	private:
		void register_runtime_operator_functions();

		YosenObject* operator_equ(YosenObject* lhs, YosenObject* rhs);
		YosenObject* operator_notequ(YosenObject* lhs, YosenObject* rhs);
		YosenObject* operator_greater(YosenObject* lhs, YosenObject* rhs);
		YosenObject* operator_less(YosenObject* lhs, YosenObject* rhs);
		YosenObject* operator_or(YosenObject* lhs, YosenObject* rhs);
		YosenObject* operator_and(YosenObject* lhs, YosenObject* rhs);
	};
}
