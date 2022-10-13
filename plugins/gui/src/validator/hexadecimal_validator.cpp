#include "gui/validator/hexadecimal_validator.h"

namespace hal
{
    HexadecimalValidator::HexadecimalValidator()
    {
        //if 0x should also be allowed, use ^(0x|0X)?[a-fA-F0-9]+$
        mValidator.setRegularExpression(QRegularExpression("^[A-F0-9]+$", QRegularExpression::CaseInsensitiveOption));
        setFailText("Cosen input must be in hexadecimal format.");
    }

    bool HexadecimalValidator::validate(const QString &input)
    {
        QString inp = input; int pos = 0;
        return (mValidator.validate(inp, pos) == QValidator::State::Acceptable) ? true : false;
    }

}
