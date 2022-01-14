#pragma once
#include "YosenObject.h"

namespace yosen
{
	class YosenString : public YosenObject
	{
	public:
		YosenString();
		YosenString(const std::string& val);

		YosenObject* clone() override;
		std::string to_string() override;
		const char* runtime_name() const override;
		
		std::string value = "";
		
	private:
		void register_member_functions();

		YosenObject* reverse(YosenObject* args);
	};
}
