#ifndef VALIDATOR_H
#define VALIDATOR_H

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

#endif      // VALIDATOR_H
