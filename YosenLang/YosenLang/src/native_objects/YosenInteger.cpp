#include "YosenInteger.h"

namespace yosen
{
	YosenObject* YosenInteger::clone()
	{
		auto new_obj = allocate_object<YosenInteger>(this->value);
		return new_obj;
	}

	std::string YosenInteger::to_string()
	{
		return std::to_string(value);
	}
	
	const char* YosenInteger::runtime_name() const
	{
		return "Integer";
	}
}
