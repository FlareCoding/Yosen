#pragma once
#include "YosenObject.h"

namespace yosen
{
	class YosenTuple : public YosenObject
	{
	public:
		YOSENAPI YosenTuple();
		YOSENAPI YosenTuple(const std::vector<YosenObject*>& items);
		YOSENAPI ~YosenTuple();

		YOSENAPI YosenObject* clone() override;
		YOSENAPI std::string to_string() override;
		YOSENAPI const char* runtime_name() const override;

		std::vector<YosenObject*> items;

	private:
		void register_member_native_functions();
	};
}
