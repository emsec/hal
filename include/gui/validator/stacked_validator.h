#ifndef STACKED_VALIDATOR_H
#define STACKED_VALIDATOR_H

#include "validator/validator.h"

#include <QList>
namespace hal{
class stacked_validator : public validator
{
    public:
        stacked_validator();

        void add_validator(validator* validator);
        void remove_validator(validator* validator);
        void clear_validators();

        bool validate(const QString &input);

    private:
        QList<validator*> m_validators;
};
}

#endif      // STACKED_VALIDATOR_H
