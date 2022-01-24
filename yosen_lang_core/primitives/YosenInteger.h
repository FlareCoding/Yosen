#pragma once
#include "YosenObject.h"

namespace yosen
{
	class YosenInteger : public YosenObject
	{
	public:
		YOSENAPI YosenInteger() = default;
		YOSENAPI YosenInteger(uint64_t val) : value(val) {}

		YOSENAPI YosenObject* clone() override;
		YOSENAPI std::string to_string() override;
		YOSENAPI const char* runtime_name() const override;

		uint64_t value = 0;
	};
}
