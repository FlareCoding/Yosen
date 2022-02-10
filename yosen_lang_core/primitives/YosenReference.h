#pragma once
#include "YosenObject.h"

namespace yosen
{
	class YosenReference : public YosenObject
	{
	public:
		YOSENAPI YosenReference();
		YOSENAPI YosenReference(YosenObject* obj);
		YOSENAPI ~YosenReference();

		YOSENAPI YosenObject* clone() override;
		YOSENAPI std::string to_string() override;
		YOSENAPI const char* runtime_name() const override;

		YosenObject* obj = nullptr;

		YOSENAPI bool has_member_native_function(const std::string& name) override;
		YOSENAPI YosenObject* call_member_native_function(const std::string& name, YosenObject* args) override;

		YOSENAPI bool has_member_runtime_function(const std::string& name) override;
		YOSENAPI ys_runtime_function_t get_member_runtime_function(const std::string& name) override;

		YOSENAPI virtual bool has_member_variable(const std::string& name) override;
		YOSENAPI virtual YosenObject* get_member_variable(const std::string& name) override;
		YOSENAPI virtual void set_member_variable(const std::string& name, YosenObject* value) override;

	private:
		void register_member_native_functions();

		YosenObject* get_obj(YosenObject* self, YosenObject* args);
	};
}
