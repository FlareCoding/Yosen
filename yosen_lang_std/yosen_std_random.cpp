#include "yosen_std_random.h"
#include <random>

using u32    = uint_least32_t; 
using engine = std::mt19937;

static std::string generate_uuid() {
    static std::random_device dev;
    static engine rng(dev());

    std::uniform_int_distribution<int> dist(0, 15);

    const char *v = "0123456789abcdef";
    const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

    std::string res;
    for (int i = 0; i < 16; i++) {
        if (dash[i]) res += "-";
        res += v[dist(rng)];
        res += v[dist(rng)];
    }
    return res;
}

static std::string generate_string(size_t length)
{
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };

    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

static std::string generate_hex_string(size_t length)
{
    char hex_characters[]= {
        '0','1','2','3','4','5','6','7',
        '8','9','A','B','C','D','E','F'
    };

    std::string result;
    for (size_t i = 0; i < length; ++i)
    {
        result += hex_characters[rand() % 16];
    }
    
    return result;
}

YosenObject* _ys_std_random_gen_int(YosenObject* args)
{
    if (static_cast<YosenTuple*>(args)->items.size())
    {
        // Free the parameter pack
		free_object(args);

		auto ex_reason = "random::gen_int() expected 0 arguments";
		YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
        return nullptr;
    }

    std::random_device os_seed;
    const u32 seed = os_seed();

    engine generator(seed);
    std::uniform_int_distribution<u32> distribute(0, std::numeric_limits<u32>::max());

    int64_t result = (int64_t)distribute(generator);
    return allocate_object<YosenInteger>(result);
}

YosenObject* _ys_std_random_gen_int_range(YosenObject* args)
{
    int64_t low = std::numeric_limits<int64_t>::min();
    int64_t high = std::numeric_limits<int64_t>::min();
	arg_parse(args, "ii", &low, &high);

	std::random_device os_seed;
    const u32 seed = os_seed();

    engine generator(seed);
    std::uniform_int_distribution<u32> distribute(static_cast<u32>(low), static_cast<u32>(high));

    int64_t result = (int64_t)distribute(generator);
    return allocate_object<YosenInteger>(result);
}

YosenObject* _ys_std_random_gen_string(YosenObject* args)
{
    int64_t length = std::numeric_limits<int64_t>::min();
	arg_parse(args, "i", &length);

    if (length <= 0)
    {
        // Free the parameter pack
		free_object(args);

		auto ex_reason = "random::gen_str() requires length to be greater than 0";
		YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
        return nullptr;
    }

    return allocate_object<YosenString>(generate_string(length));
}

YosenObject* _ys_std_random_gen_hex_string(YosenObject* args)
{
    int64_t length = std::numeric_limits<int64_t>::min();
	arg_parse(args, "i", &length);

    if (length <= 0)
    {
        // Free the parameter pack
		free_object(args);

		auto ex_reason = "random::gen_hex_str() requires length to be greater than 0";
		YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
        return nullptr;
    }

    return allocate_object<YosenString>(generate_hex_string(length));
}

YosenObject* _ys_std_random_gen_uuid(YosenObject* args)
{
    if (static_cast<YosenTuple*>(args)->items.size())
    {
        // Free the parameter pack
		free_object(args);

		auto ex_reason = "random::gen_uuid() expected 0 arguments";
		YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
        return nullptr;
    }
    
    return allocate_object<YosenString>(generate_uuid());
}
