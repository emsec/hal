#include "validator/empty_string_validator.h"

namespace hal
{
    EmptyStringValidator::EmptyStringValidator()
    {
        set_fail_text("Chosen input can't be empty.");
    }

    bool EmptyStringValidator::validate(const QString &input)
    {
        return !input.trimmed().isEmpty();
    }
}
