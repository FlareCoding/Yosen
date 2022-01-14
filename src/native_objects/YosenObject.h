#pragma once
#include <string>
#include <functional>
#include <map>

//#define PROFILE_OBJECT_ALLOCATION

namespace yosen
{
	class YosenObject;
	using yosen_function_t = std::function<YosenObject*(YosenObject*)>;

	class YosenObject
	{
		friend class Interpreter;

	public:
		YosenObject();
		virtual ~YosenObject() = default;

		// Creates and returns a value copy of the object
		virtual YosenObject* clone();

		// Returns a string representation of the object
		virtual std::string to_string();

		// Returns the runtime type of the object
		virtual const char* runtime_name() const;

		// Adds a member function to the object
		void add_member_function(const std::string& name, yosen_function_t fn);

		// Returns whether or not the object has a member function with the given name
		bool has_member_function(const std::string& name);

		// Returns the native pointer to a member function
		yosen_function_t get_member_function(const std::string& name);

	protected:
		std::string string_repr;

	private:
		std::map<std::string, yosen_function_t> member_functions;
	};

#define MEMBER_FUNCTION(fn) [this](YosenObject* args) { return fn(args); }

	void __yosen_register_allocated_object(void* obj);
	uint64_t __yosen_get_total_allocated_objects();

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
	
	void free_object(YosenObject* obj);
	bool arg_parse(YosenObject* obj, const char* fmt, ...);

	extern YosenObject* YosenObject_Null;
}
