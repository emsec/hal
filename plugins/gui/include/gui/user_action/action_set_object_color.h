#pragma once
#include "user_action.h"
#include <QColor>

namespace hal
{
    /**
     * @ingroup gui
     */
    class ActionSetObjectColor : public UserAction
    {
        QColor mColor;
    public:
        ActionSetObjectColor(const QColor& col=QColor()) : mColor(col) {;}
        bool exec() override;
        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void addToHash(QCryptographicHash& cryptoHash) const override;
    };

    /**
     * @ingroup gui
     */
    class ActionSetObjectColorFactory : public UserActionFactory
    {
    public:
        ActionSetObjectColorFactory();
        UserAction* newAction() const;
        static ActionSetObjectColorFactory* sFactory;
    };
}
