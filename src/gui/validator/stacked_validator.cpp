#include "validator/stacked_validator.h"
namespace hal{
stacked_validator::stacked_validator() : m_validators(QList<validator*>())
{

}

void stacked_validator::add_validator(validator* v)
{
    m_validators.append(v);
}

void stacked_validator::remove_validator(validator* v)
{
    m_validators.removeAll(v);
}

void stacked_validator::clear_validators()
{
    m_validators.clear();
}

bool stacked_validator::validate(const QString &input)
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
