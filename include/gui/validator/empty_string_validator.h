#ifndef EMPTY_STRING_VALIDATOR_H
#define EMPTY_STRING_VALIDATOR_H

#include "validator/validator.h"

class empty_string_validator : public validator
{
    public:
        empty_string_validator();

        bool validate(const QString &input);
};

#endif      // EMPTY_STRING_VALIDATOR_H
