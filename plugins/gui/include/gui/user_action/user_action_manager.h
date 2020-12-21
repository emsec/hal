#pragma once
#include <QObject>
#include <QList>

namespace hal
{
    class UserAction;

    class UserActionManager : public QObject
    {
        Q_OBJECT
    public:
        enum UserActionType { NoAction,
                              OpenNetlistFile, NewSelection, SelectionToModule, SelectionToGrouping};
        void addExecutedAction(UserAction* act);
        static UserActionManager* instance();
    private:
        UserActionManager(QObject *parent = nullptr);

        QList<UserAction*> mActionHistory;
        static UserActionManager* inst;

    Q_SIGNALS:

    };
}
