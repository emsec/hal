#include "gui/validator/stacked_validator.h"
namespace hal
{
    StackedValidator::StackedValidator() : mValidators(QList<Validator*>())
    {

    }

    void StackedValidator::addValidator(Validator* v)
    {
        mValidators.append(v);
    }

    void StackedValidator::removeValidator(Validator* v)
    {
        mValidators.removeAll(v);
    }

    void StackedValidator::clearValidators()
    {
        mValidators.clear();
    }

    bool StackedValidator::validate(const QString &input)
    {
        for(const auto& v : mValidators)
        {
            if(v != nullptr && !v->validate(input))
            {
                setFailText(v->failText());
                return false;
            }
        }

        return true;
    }
}
