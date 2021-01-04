#pragma once
#include <QObject>
#include <QList>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include "hal_core/defines.h"

namespace hal
{
    class UserAction;
    class GraphContext;

    class UserActionManager : public QObject
    {
        Q_OBJECT
    public:
        enum UserActionType { NoAction, CompoundAction,
                              OpenNetlistFile, UnfoldModule, NewSelection,
                              CreateGrouping, DeleteGrouping, SelectionToGrouping,
                              CreateModule, SelectionToModule};
        Q_ENUM(UserActionType)

        void addExecutedAction(UserAction* act);
        static UserActionManager* instance();
        void setStartRecording();
        void setStopRecording(const QString& macroFilename);
        void playMacro(const QString& macroFilename);
        bool isRecording() const;
        bool hasRecorded() const;
        void clearWaitCount() { mWaitCount = 0; }
        static UserAction* getParsedAction(QXmlStreamReader& xmlIn);
    private:
        UserActionManager(QObject *parent = nullptr);

        QList<UserAction*> mActionHistory;
        int mStartRecording;
        int mWaitCount;

        static UserActionManager* inst;

    Q_SIGNALS:

    };
}
