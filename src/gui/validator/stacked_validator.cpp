#include "validator/stacked_validator.h"
namespace hal
{
    StackedValidator::StackedValidator() : m_validators(QList<Validator*>())
    {

    }

    void StackedValidator::add_validator(Validator* v)
    {
        m_validators.append(v);
    }

    void StackedValidator::remove_validator(Validator* v)
    {
        m_validators.removeAll(v);
    }

    void StackedValidator::clear_validators()
    {
        m_validators.clear();
    }

    bool StackedValidator::validate(const QString &input)
    {
        for(const auto& v : m_validators)
        {
            if(v != nullptr && !v->validate(input))
            {
                set_fail_text(v->fail_text());
                return false;
            }
        }

        return true;
    }
}
