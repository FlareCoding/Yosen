#pragma once
#include <YosenEnvironment.h>
using namespace yosen;

// Generates and returns a random integer
YosenObject* _ys_std_random_gen_int(YosenObject* args);

// Generates and returns a random integer,
// within range [low, high] inclusive.
YosenObject* _ys_std_random_gen_int_range(YosenObject* args);

// Generates and returns a random string of given length
YosenObject* _ys_std_random_gen_string(YosenObject* args);

// Generates and returns a random hex string
YosenObject* _ys_std_random_gen_hex_string(YosenObject* args);

// Generates and returns a new unique UUID
YosenObject* _ys_std_random_gen_uuid(YosenObject* args);
