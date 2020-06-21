#include "validator/validator.h"

namespace hal
{
    void Validator::set_fail_text(const QString& text)
    {
        m_fail_text = text;
    }

    QString Validator::fail_text() const
    {
        return m_fail_text;
    }
}
