#include "validator/unique_string_validator.h"

namespace hal
{
    UniqueStringValidator::UniqueStringValidator(const QStringList &unique_strings) : m_unique_strings(unique_strings)
    {
        set_fail_text("Chosen input is already assigned.");
    }

    bool UniqueStringValidator::validate(const QString &input)
    {
        if(!m_unique_strings.contains(input))
            return true;
        else
            return false;
    }
}
