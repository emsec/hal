#include "validator/stacked_validator.h"

stacked_validator::stacked_validator() : m_validators(QList<validator*>())
{  
    
}

void stacked_validator::add_validator(validator* validator)
{
    m_validators.append(validator);
}

void stacked_validator::remove_validator(validator* validator)
{
    m_validators.removeAll(validator);
}

void stacked_validator::clear_validators()
{
    m_validators.clear();
}

bool stacked_validator::validate(const QString &input)
{
    for(validator* v : m_validators)
    {
        if(v != nullptr && !v->validate(input))
        {
            set_fail_text(v->fail_text());
            return false;
        }
    }
    
    return true;
}
