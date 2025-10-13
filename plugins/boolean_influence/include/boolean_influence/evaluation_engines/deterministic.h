
#include <string>

namespace hal
{
    namespace boolean_influence
    {
        namespace
        {

            const std::string deterministic_function = R"(
#include <stdbool.h> 
#include <stdio.h>
#include <stdlib.h>

<C_FUNCTION>

const int input_size = <INPUT_SIZE>;

void build_values(bool* values, unsigned long long val) {
    for (int idx = 0; idx < input_size; idx++) {
        values[idx] = (val >> idx) & 0x1;
    }

    return;
}

int main(int argc, char *argv[]) {
    unsigned long long b     = strtoull(argv[1], 0, 10);
    unsigned long long count = 0;

    bool values[input_size];
    for (unsigned long long i = 0; i < (1 << input_size); i++) {
        build_values(values, i);

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