#pragma once
#include "YosenObject.h"

namespace yosen
{
	class YosenString : public YosenObject
	{
	public:
		YOSENAPI YosenString();
		YOSENAPI YosenString(const std::string& val);

		YOSENAPI YosenObject* clone() override;
		YOSENAPI std::string to_string() override;
		YOSENAPI const char* runtime_name() const override;
		
		std::string value = "";
		
	private:
		void register_member_native_functions();

		YosenObject* length(YosenObject* self, YosenObject* args);
		YosenObject* reverse(YosenObject* self, YosenObject* args);
		YosenObject* append(YosenObject* self, YosenObject* args);
		YosenObject* substr(YosenObject* self, YosenObject* args);
		YosenObject* contains(YosenObject* self, YosenObject* args);
		YosenObject* find(YosenObject* self, YosenObject* args);
		YosenObject* remove(YosenObject* self, YosenObject* args);
		YosenObject* clear(YosenObject* self, YosenObject* args);
		YosenObject* is_empty(YosenObject* self, YosenObject* args);

	private:
		void register_runtime_operator_functions();

		YosenObject* operator_add(YosenObject* lhs, YosenObject* rhs);
		YosenObject* operator_equ(YosenObject* lhs, YosenObject* rhs);
		YosenObject* operator_notequ(YosenObject* lhs, YosenObject* rhs);
	};
}
