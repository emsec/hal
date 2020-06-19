#pragma once

#include "validator/validator.h"

#include <QStringList>
namespace hal{
class unique_string_validator : public validator
{
    public:
        unique_string_validator(const QStringList &unique_strings);

        bool validate(const QString &input);

    private:
        const QStringList &m_unique_strings;
};
}
