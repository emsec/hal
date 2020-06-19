#pragma once

#include "validator/validator.h"
namespace hal{
class empty_string_validator : public validator
{
    public:
        empty_string_validator();

        bool validate(const QString &input);
};
}
