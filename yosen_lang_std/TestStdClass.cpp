#include "TestStdClass.h"

YosenObject* test_std_class_builder(YosenObject* args)
{
	return allocate_object<TestStdClass>();
}

YosenObject* _ys_std_println(YosenObject* args)
{
	YosenObject* obj;
	arg_parse(args, "o", &obj);

	printf("%s\n", obj->to_string().c_str());
	return YosenObject_Null->clone();
}

TestStdClass::TestStdClass()
{
	add_member_native_function("test_fn", MEMBER_FUNCTION(test_fn));
}

YosenObject* TestStdClass::clone()
{
	return allocate_object<TestStdClass>();
}

std::string TestStdClass::to_string()
{
	return "testclass";
}

const char* TestStdClass::runtime_name() const
{
	return "TestStdClass";
}

YosenObject* TestStdClass::test_fn(YosenObject* self, YosenObject* args)
{
	printf("YEET TEST WORKING!\n");
	return YosenObject_Null->clone();
}
