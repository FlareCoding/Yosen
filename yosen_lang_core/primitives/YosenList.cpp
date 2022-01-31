#include "YosenList.h"
#include "YosenInteger.h"

namespace yosen
{
	YosenList::YosenList()
	{
		register_member_native_functions();
	}

	YosenList::YosenList(const std::vector<YosenObject*>& items)
		: items(items)
	{
		register_member_native_functions();
	}

	YosenList::~YosenList()
	{
		for (auto& item : items)
			free_object(item);
	}

	YosenObject* YosenList::clone()
	{
		auto new_obj = allocate_object<YosenList>();
		for (auto& item : items)
			new_obj->items.push_back(item->clone());

		return new_obj;
	}

	std::string YosenList::to_string()
	{
		std::string result = "[";
		for (size_t i = 0; i < items.size(); i++)
		{
			result += items[i]->to_string();

			if (i <= items.size() - 2 && items.size() > 1)
				result += ", ";
		}

		result += "]";
		return result;
	}

	const char* YosenList::runtime_name() const
	{
		return "List";
	}

	void YosenList::register_member_native_functions()
	{
		add_member_native_function("get",		MEMBER_FUNCTION(get));
		add_member_native_function("add",		MEMBER_FUNCTION(add));
		add_member_native_function("remove",	MEMBER_FUNCTION(remove));
		add_member_native_function("clear",		MEMBER_FUNCTION(clear));
		add_member_native_function("length",	MEMBER_FUNCTION(length));
	}
	
	YosenObject* YosenList::get(YosenObject* self, YosenObject* args)
	{
		int64_t index;
		arg_parse(args, "i", &index);

		auto this_obj = static_cast<YosenList*>(self);
		return this_obj->items[index]->clone();
	}
	
	YosenObject* YosenList::add(YosenObject* self, YosenObject* args)
	{
		YosenObject* obj;
		arg_parse(args, "o", &obj);

		auto this_obj = static_cast<YosenList*>(self);
		this_obj->items.push_back(obj->clone());

		return YosenObject_Null->clone();
	}
	
	YosenObject* YosenList::remove(YosenObject* self, YosenObject* args)
	{
		int64_t index;
		arg_parse(args, "i", &index);

		auto this_obj = static_cast<YosenList*>(self);
		this_obj->items.erase(this_obj->items.begin() + index);

		return YosenObject_Null->clone();
	}
	
	YosenObject* YosenList::clear(YosenObject* self, YosenObject* args)
	{
		auto this_obj = static_cast<YosenList*>(self);

		for (auto& item : this_obj->items)
			free_object(item);

		this_obj->items.clear();

		return YosenObject_Null->clone();
	}
	
	YosenObject* YosenList::length(YosenObject* self, YosenObject* args)
	{
		auto this_obj = static_cast<YosenList*>(self);

		return allocate_object<YosenInteger>((int64_t)this_obj->items.size());
	}
}
