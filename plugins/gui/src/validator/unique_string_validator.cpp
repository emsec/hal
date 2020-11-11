#include "gui/validator/unique_string_validator.h"

namespace hal
{
    UniqueStringValidator::UniqueStringValidator(const QStringList &unique_strings) : mUniqueStrings(unique_strings)
    {
        setFailText("Chosen input is already assigned.");
    }

    bool UniqueStringValidator::validate(const QString &input)
    {
        if(!mUniqueStrings.contains(input))
            return true;
        else
            return false;
    }
}
