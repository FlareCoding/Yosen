#pragma once
#include "YosenObject.h"

namespace yosen
{
	class YosenList : public YosenObject
	{
	public:
		YOSENAPI YosenList();
		YOSENAPI YosenList(const std::vector<YosenObject*>& items);
		YOSENAPI ~YosenList();

		YOSENAPI YosenObject* clone() override;
		YOSENAPI std::string to_string() override;
		YOSENAPI const char* runtime_name() const override;

		std::vector<YosenObject*> items;

	private:
		void register_member_native_functions();

		YosenObject* get(YosenObject* self, YosenObject* args);
		YosenObject* add(YosenObject* self, YosenObject* args);
		YosenObject* remove(YosenObject* self, YosenObject* args);
		YosenObject* clear(YosenObject* self, YosenObject* args);
		YosenObject* length(YosenObject* self, YosenObject* args);
		YosenObject* contains(YosenObject* self, YosenObject* args);
		YosenObject* find(YosenObject* self, YosenObject* args);
		YosenObject* is_empty(YosenObject* self, YosenObject* args);
		YosenObject* slice(YosenObject* self, YosenObject* args);
		YosenObject* first(YosenObject* self, YosenObject* args);
		YosenObject* last(YosenObject* self, YosenObject* args);
		YosenObject* pop_back(YosenObject* self, YosenObject* args);
	};
}
