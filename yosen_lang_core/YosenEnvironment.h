#pragma once
#include <utils/utils.h>

// Primitive Types
#include <primitives/YosenTuple.h>
#include <primitives/YosenInteger.h>
#include <primitives/YosenString.h>

namespace yosen
{
	class YosenEnvironment
	{
	public:
		YOSENAPI static void init();
		YOSENAPI void shutdown();
		YOSENAPI static YosenEnvironment& get();

		YOSENAPI
		void register_static_native_function(
			const std::string& name,
			ys_static_native_fn_t fn
		);

		YOSENAPI
		bool is_static_native_function(
			const std::string& name
		);

		YOSENAPI
		ys_static_native_fn_t get_static_native_function(
			const std::string& name
		);

		YOSENAPI
		bool load_yosen_module(
			const std::string& name
		);

		YOSENAPI
		void register_yosen_class(
			const std::string& name,
			ys_class_builder_fn_t builder_fn
		);

		YOSENAPI
		bool is_class_name(
			const std::string& name
		);

		YOSENAPI
		YosenObject* construct_class_instance(
			const std::string& name
		);

		YOSENAPI
		void start_module_namespace(
			const std::string& name
		);

		YOSENAPI
		void end_module_namespace();

	private:
		void initialize_standard_library();

	private:
		// Empty string denotes global namespace
		std::string m_current_module_namespace = "";

		std::map<std::string, ys_static_native_fn_t> m_static_native_functions;
		std::map<std::string, ys_class_builder_fn_t> m_custom_class_builders;
	};
}
