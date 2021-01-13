//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once
#include <QObject>
#include <QList>
#include <QHash>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QElapsedTimer>
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
        /// get singleton instance
        static UserActionManager* instance();

        /// add executed action to history list
        void addExecutedAction(UserAction* act);

        /// register new action factory
        void registerFactory(UserActionFactory* fac);

        /// set start marker for macro recording
        void setStartRecording();

        /// stop macro recording and write commands to xml file
        void setStopRecording(const QString& macroFilename);

        /// execute macro from file
        void playMacro(const QString& macroFilename);

        /// test whether actions are currently recorded
        bool isRecording() const;

        /// test whether actions are currently recorded and
        /// at least one command has been executed
        bool hasRecorded() const;

        /// reset wait flag issued from action
        void clearWaitCount() { mWaitCount = 0; }

        /// create UserAction instance for next command found by xml parser
        UserAction* getParsedAction(QXmlStreamReader& xmlIn) const;

        /// elapsed time in milliseconds since GUI start
        qint64 timeStamp() const { return mElapsedTime.elapsed(); }
    private:
        UserActionManager(QObject *parent = nullptr);

        QList<UserAction*> mActionHistory;
        QHash<QString,UserActionFactory*> mActionFactory;
        int mStartRecording;
        int mWaitCount;
        QElapsedTimer mElapsedTime;
        static UserActionManager* inst;

    Q_SIGNALS:

    };
}
