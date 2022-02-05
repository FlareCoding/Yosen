#include "RuntimeClassBuilder.h"
#include <YosenEnvironment.h>

namespace yosen
{
	void RuntimeClassBuilder::create_runtime_class()
	{
		YosenEnvironment::get().register_yosen_class(class_name, [this](YosenObject* args) -> YosenObject* {
			auto instance = allocate_object<YosenObject>();
			instance->override_runtime_name(class_name);

			// Add member functions
			for (auto& [name, fn] : runtime_functions)
				instance->add_member_runtime_function(name, fn);

			// Add member variables
			for (auto& [name, value] : member_variables)
				instance->add_member_variable(name, value);

			return instance;
		});
	}
}
