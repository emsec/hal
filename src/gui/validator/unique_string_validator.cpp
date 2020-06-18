#include "validator/unique_string_validator.h"
namespace hal{
unique_string_validator::unique_string_validator(const QStringList &unique_strings) : m_unique_strings(unique_strings)
{
    set_fail_text("Choosen input is already assigned.");
}

bool unique_string_validator::validate(const QString &input)
{
    if(!m_unique_strings.contains(input))
        return true;
    else
        return false;
}
}
