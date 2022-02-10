#pragma once
#include <YosenCore.h>
#include <string>
#include <functional>
#include <map>

//#define PROFILE_OBJECT_ALLOCATION

namespace yosen
{
	class StackFrame;
	class YosenObject;

	using ys_static_native_fn_t		= std::function<YosenObject* (YosenObject*)>;
	using ys_member_native_fn_t		= std::function<YosenObject* (YosenObject*, YosenObject*)>;
	using ys_class_builder_fn_t		= std::function<YosenObject* (YosenObject*)>;
	using ys_runtime_function_t		= std::pair<std::shared_ptr<StackFrame>, std::vector<unsigned short>>;
	using ys_runtime_operator_fn_t	= std::function<YosenObject* (YosenObject*, YosenObject*)>;

	enum class RuntimeOperator
	{
		BinOpAdd,
		BinOpSub,
		BinOpMul,
		BinOpDiv,
		BinOpMod,
		BoolOpEqu,
		BoolOpNotEqu,
		BoolOpGreaterThan,
		BoolOpLessThan,
		BoolOpOr,
		BoolOpAnd,
	};

	class YosenObject
	{
		friend class YosenEnvironment;

	public:
		YOSENAPI YosenObject();
		YOSENAPI virtual ~YosenObject();

		// Creates and returns a value copy of the object
		YOSENAPI virtual YosenObject* clone();

		// Returns a string representation of the object
		YOSENAPI virtual std::string to_string();

		// Returns the runtime type of the object
		YOSENAPI virtual const char* runtime_name() const;

		// Returns information about the instance
		YOSENAPI std::string instance_info() const;

		// Adds a member function to the object
		YOSENAPI virtual void add_member_native_function(const std::string& name, ys_member_native_fn_t fn);

		// Returns whether or not the object has a member function with the given name
		YOSENAPI virtual bool has_member_native_function(const std::string& name);

		// Calls the native member function
		YOSENAPI virtual YosenObject* call_member_native_function(const std::string& name, YosenObject* args);

		// Adds a member function to the object
		YOSENAPI virtual void add_member_runtime_function(const std::string& name, ys_runtime_function_t fn);

		// Returns whether or not the object has a member function with the given name
		YOSENAPI virtual bool has_member_runtime_function(const std::string& name);

		// Returns a member runtime function with the given name
		YOSENAPI virtual ys_runtime_function_t get_member_runtime_function(const std::string& name);

		// Adds a member variable to the object
		YOSENAPI virtual void add_member_variable(const std::string& name, YosenObject* value);

		// Returns whether or not the object has a member variable with the given name
		YOSENAPI virtual bool has_member_variable(const std::string& name);

		// Returns a member variable with the given name
		YOSENAPI virtual YosenObject* get_member_variable(const std::string& name);

		// Sets the member variable given the name
		YOSENAPI virtual void set_member_variable(const std::string& name, YosenObject* value);

		// Adds a runtime operator function to the object
		YOSENAPI void add_runtime_operator_function(RuntimeOperator op, ys_runtime_operator_fn_t fn);

		// Calls an appropriate function according to the provided runtime operator
		YOSENAPI YosenObject* call_runtime_operator_function(RuntimeOperator op, YosenObject* rhs);

		//
		// The following methods are used for overriding
		// native functionality with runtime functions.
		//
		YOSENAPI void override_runtime_name(const std::string& name) { m_overriden_runtime_name = name; }
		YOSENAPI void override_to_string_repr(const std::string& repr) { m_string_repr = repr; }

	protected:
		std::string m_string_repr;

	protected:
		std::string m_overriden_runtime_name;

		std::map<std::string, ys_member_native_fn_t> m_member_native_functions;
		std::map<std::string, ys_runtime_function_t> m_member_runtime_functions;

		std::map<RuntimeOperator, ys_runtime_operator_fn_t> m_runtime_operator_functions;

		std::map<std::string, YosenObject*> m_member_variables;
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
