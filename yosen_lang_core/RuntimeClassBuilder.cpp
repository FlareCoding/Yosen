#include "RuntimeClassBuilder.h"
#include <YosenEnvironment.h>

namespace yosen
{
	void RuntimeClassBuilder::create_runtime_class()
	{
		YosenEnvironment::get().register_yosen_class(class_name, [this](YosenObject* args) -> YosenObject* {
			auto instance = allocate_object<YosenObject>();
			instance->override_runtime_name(class_name);

			for (auto& [name, fn] : runtime_functions)
				if (!name._Equal("new"))
					instance->add_member_runtime_function(name, fn);

			return instance;
		});
	}
}
