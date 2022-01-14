#pragma once
#include "YosenObject.h"

namespace yosen
{
	class YosenInteger : public YosenObject
	{
	public:
		YosenInteger() = default;
		YosenInteger(uint64_t val) : value(val) {}

		YosenObject* clone() override;
		std::string to_string() override;
		const char* runtime_name() const override;

		uint64_t value = 0;
	};
}
