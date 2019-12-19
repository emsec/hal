#include "validator/validator.h"

void validator::set_fail_text(const QString& text)
{
    m_fail_text = text;
}

QString validator::fail_text() const
{
    return m_fail_text;
}
