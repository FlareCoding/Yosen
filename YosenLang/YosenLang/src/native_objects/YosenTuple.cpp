#include "YosenTuple.h"

namespace yosen
{
	YosenTuple::YosenTuple()
	{
		register_member_functions();
	}

	YosenTuple::YosenTuple(const std::vector<YosenObject*>& items)
		: items(items)
	{
		register_member_functions();
	}

	YosenTuple::~YosenTuple()
	{
		for (auto& item : items)
			free_object(item);
	}

	YosenObject* YosenTuple::clone()
	{
		auto new_obj = allocate_object<YosenTuple>();
		for (auto& item : items)
			new_obj->items.push_back(item->clone());

		return new_obj;
	}

	std::string YosenTuple::to_string()
	{
		std::string result = "(";
		for (size_t i = 0; i < items.size(); i++)
		{
			result += items[i]->to_string();

			if (i == items.size() - 2)
				result += ", ";
		}
		
		result += ")";
		return result;
	}

	const char* YosenTuple::runtime_name() const
	{
		return "Tuple";
	}

	void YosenTuple::register_member_functions()
	{
	}
}
