#pragma once
#include <YosenCore.h>
#include <string>
#include <functional>
#include <map>

//#define PROFILE_OBJECT_ALLOCATION

namespace yosen
{
	class YosenObject;
	using ys_static_native_fn_t		= std::function<YosenObject* (YosenObject*)>;
	using ys_member_native_fn_t		= std::function<YosenObject* (YosenObject*, YosenObject*)>;
	using ys_class_builder_fn_t		= std::function<YosenObject* (YosenObject*)>;

	class YosenObject
	{
		friend class YosenEnvironment;

	public:
		YOSENAPI YosenObject();
		YOSENAPI virtual ~YosenObject() = default;

		// Creates and returns a value copy of the object
		YOSENAPI virtual YosenObject* clone();

		// Returns a string representation of the object
		YOSENAPI virtual std::string to_string();

		// Returns the runtime type of the object
		YOSENAPI virtual const char* runtime_name() const;

		// Adds a member function to the object
		YOSENAPI void add_member_native_function(const std::string& name, ys_member_native_fn_t fn);

		// Returns whether or not the object has a member function with the given name
		YOSENAPI bool has_member_native_function(const std::string& name);

		// Calls the native member function
		YOSENAPI YosenObject* call_member_native_function(const std::string& name, YosenObject* args);

	protected:
		std::string m_string_repr;

	private:
		std::map<std::string, ys_member_native_fn_t> m_member_native_functions;
	};

#define MEMBER_FUNCTION(fn) [this](YosenObject* self, YosenObject* args) { return fn(self, args); }

	YOSENAPI void __yosen_register_allocated_object(void* obj);
	YOSENAPI uint64_t __yosen_get_total_allocated_objects();

	template<typename T, typename ...Args>
	T* allocate_object(Args && ...args)
	{
		T* object = new T(std::forward<Args>(args)...);
		__yosen_register_allocated_object(object);

#ifdef PROFILE_OBJECT_ALLOCATION
		printf("YosenObject Allocated,  total object count: %zi\n", __yosen_get_total_allocated_objects());
#endif // PROFILE_OBJECT_ALLOCATION

		return object;
	}
	
	YOSENAPI void free_object(YosenObject* obj);
	YOSENAPI bool arg_parse(YosenObject* obj, const char* fmt, ...);

	YOSENAPI extern YosenObject* YosenObject_Null;
}
