#pragma once
#include "YosenObject.h"

namespace yosen
{
	class YosenTuple : public YosenObject
	{
	public:
		YosenTuple();
		YosenTuple(const std::vector<YosenObject*>& items);
		~YosenTuple();

		YosenObject* clone() override;
		std::string to_string() override;
		const char* runtime_name() const override;

		std::vector<YosenObject*> items;

	private:
		void register_member_functions();
	};
}
