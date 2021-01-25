#pragma once
#include "user_action.h"
#include <QPoint>
#include <QString>

namespace hal
{
    class ActionMoveNode : public UserAction
    {
        QPoint mFrom, mTo;

        static QPoint parseFromString(const QString& s);
    public:
        ActionMoveNode(const QPoint& from = QPoint(), const QPoint& to = QPoint())
            : mFrom(from), mTo(to) {;}
        void exec() override;
        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void addToHash(QCryptographicHash& cryptoHash) const override;
    };

    class ActionMoveNodeFactory : public UserActionFactory
    {
    public:
        ActionMoveNodeFactory();
        UserAction* newAction() const;
        static ActionMoveNodeFactory* sFactory;
    };
}
