#include "YosenList.h"
#include <YosenEnvironment.h>
#include <algorithm>

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
		add_member_native_function("contains",	MEMBER_FUNCTION(contains));
		add_member_native_function("find",		MEMBER_FUNCTION(find));
		add_member_native_function("is_empty",	MEMBER_FUNCTION(is_empty));
		add_member_native_function("slice",		MEMBER_FUNCTION(slice));
		add_member_native_function("first",		MEMBER_FUNCTION(first));
		add_member_native_function("last",		MEMBER_FUNCTION(last));
		add_member_native_function("pop_back",	MEMBER_FUNCTION(pop_back));
	}
	
	YosenObject* YosenList::get(YosenObject* self, YosenObject* args)
	{
		int64_t index;
		arg_parse(args, "i", &index);

		auto this_obj = static_cast<YosenList*>(self);

		if (index < 0 || index >= (int64_t)this_obj->items.size())
		{
			auto ex_reason = "list index out of bounds";
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

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

		if (index < 0 || index >= (int64_t)this_obj->items.size())
		{
			auto ex_reason = "list index out of bounds";
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

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

	YosenObject* YosenList::contains(YosenObject* self, YosenObject* args)
	{
		YosenObject* target = nullptr;
		arg_parse(args, "o", &target);

		if (!target)
			return nullptr;

		auto this_obj = static_cast<YosenList*>(self);

		bool result = std::find(this_obj->items.begin(), this_obj->items.end(), target) != this_obj->items.end();
		return allocate_object<YosenBoolean>(result);
	}

	YosenObject* YosenList::find(YosenObject* self, YosenObject* args)
	{
		YosenObject* target = nullptr;
		arg_parse(args, "o", &target);

		if (!target)
			return nullptr;

		auto this_obj = static_cast<YosenList*>(self);

		auto it = std::find(this_obj->items.begin(), this_obj->items.end(), target);
		
		int64_t result = -1;
		if (it != this_obj->items.end())
			result = (int64_t)(it - this_obj->items.begin());

		return allocate_object<YosenInteger>(result);
	}

	YosenObject* YosenList::is_empty(YosenObject* self, YosenObject* args)
	{
		auto this_obj = static_cast<YosenList*>(self);
		bool result = this_obj->items.empty();

		return allocate_object<YosenBoolean>(result);
	}

	YosenObject* YosenList::slice(YosenObject* self, YosenObject* args)
	{
		int64_t start = 0;
		int64_t end = 0;
		arg_parse(args, "ii", &start, &end);

		auto this_obj = static_cast<YosenList*>(self);

		if (start < 0 || start >= (int64_t)this_obj->items.size())
		{
			auto ex_reason = "list::slice() - start index out of range";
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		if (end < 0 || end >= (int64_t)this_obj->items.size())
		{
			auto ex_reason = "list::slice() - end index out of range";
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		std::vector<YosenObject*> resulting_list;
		for (int64_t i = start; i < end; ++i)
			resulting_list.push_back(this_obj->items.at((size_t)i)->clone());

		return allocate_object<YosenList>(resulting_list);
	}

	YosenObject* YosenList::first(YosenObject* self, YosenObject* args)
	{
		auto this_obj = static_cast<YosenList*>(self);

		if (!this_obj->items.size())
		{
			auto ex_reason = "list::first() - list is empty";
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		return this_obj->items.at(0)->clone();
	}

	YosenObject* YosenList::last(YosenObject* self, YosenObject* args)
	{
		auto this_obj = static_cast<YosenList*>(self);

		if (!this_obj->items.size())
		{
			auto ex_reason = "list::first() - list is empty";
			YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
			return nullptr;
		}

		return this_obj->items.back()->clone();
	}

	YosenObject* YosenList::pop_back(YosenObject* self, YosenObject* args)
	{
		auto this_obj = static_cast<YosenList*>(self);
		this_obj->items.pop_back();

		return YosenObject_Null->clone();
	}
}
