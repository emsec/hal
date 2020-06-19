#pragma once

#include <QString>
namespace hal{
class validator
{
    public:
        virtual bool validate(const QString &input) = 0;

        void set_fail_text(const QString& text);
        QString fail_text() const;

    protected:
        QString m_fail_text = "Choosen input is not allowed.";
};
}
