#include "validator/empty_string_validator.h"

namespace hal
{
    empty_string_validator::empty_string_validator()
    {
        set_fail_text("Choosen input can't be empty.");
    }

    bool empty_string_validator::validate(const QString &input)
    {
        return !input.trimmed().isEmpty();
    }
}
