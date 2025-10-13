
#include <string>

namespace hal
{
    namespace boolean_influence
    {
        namespace
        {
            const std::string probabilistic_function = R"(
#include <stdbool.h> 
#include <stdio.h>
#include <stdlib.h>

static unsigned long x=123456789, y=362436069, z=521288629;

// period 2^96-1
unsigned long xorshf96(void) {          
    unsigned long t;

    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

    t = x;
    x = y;
    y = z;
    z = t ^ x ^ y;

    return z;
}

<C_FUNCTION>

const int input_size = <INPUT_SIZE>;

void build_values(bool* values) {
    for (int i = 0; i < input_size; i++) {
        bool random_value = xorshf96() % 2;
        values[i] = random_value;
    }

    return;
}

int main(int argc, char *argv[]) {
    unsigned long long b     = strtoull(argv[1], 0, 10);
    unsigned long long num   = strtoull(argv[2], 0, 10);
    unsigned long long count = 0;

    bool values[input_size];
    for (unsigned long long i = 0; i < num; i++) {
        build_values(values);

        values[b] = true;
        bool r1 = func(values);

        values[b] = false;
        bool r2 = func(values);

        if (r1 != r2) {
            count++;
        }
    }

    printf("%lld\n", count);

    return int(count);
})";
        }
    }    // namespace boolean_influence
}    // namespace hal