#pragma once
#include <YosenEnvironment.h>
using namespace yosen;

class TestStdClass : public YosenObject
{
public:
	TestStdClass();

	YosenObject* clone() override;
	std::string to_string() override;
	const char* runtime_name() const override;

private:
	YosenObject* test_fn(YosenObject* self, YosenObject* args);
};

YosenObject* test_std_class_builder();

YosenObject* _ys_std_println(YosenObject* args);
