#pragma once
#include <QObject>
#include <QList>
#include <QHash>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include "hal_core/defines.h"

namespace hal
{
    class UserAction;
    class UserActionFactory;
    class GraphContext;

    class UserActionManager : public QObject
    {
        Q_OBJECT
    public:
        void addExecutedAction(UserAction* act);
        void registerFactory(UserActionFactory* fac);

        static UserActionManager* instance();
        void setStartRecording();
        void setStopRecording(const QString& macroFilename);
        void playMacro(const QString& macroFilename);
        bool isRecording() const;
        bool hasRecorded() const;
        void clearWaitCount() { mWaitCount = 0; }
        UserAction* getParsedAction(QXmlStreamReader& xmlIn) const;
    private:
        UserActionManager(QObject *parent = nullptr);

        QList<UserAction*> mActionHistory;
        QHash<QString,UserActionFactory*> mActionFactory;
        int mStartRecording;
        int mWaitCount;

        static UserActionManager* inst;

    Q_SIGNALS:

    };
}
