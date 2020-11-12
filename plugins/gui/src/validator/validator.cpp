#include "gui/validator/validator.h"

namespace hal
{
    void Validator::setFailText(const QString& text)
    {
        mFailText = text;
    }

    QString Validator::failText() const
    {
        return mFailText;
    }
}
