// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once
#include <QObject>
#include <QList>
#include <QHash>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QElapsedTimer>
#include <QPlainTextEdit>
#include <QMessageBox>
#include "hal_core/defines.h"

namespace hal
{
    class UserAction;
    class UserActionFactory;
    class GraphContext;
    class SettingsItemCheckbox;

    /**
     * @ingroup user_action
     * @brief Handles and manages user actions related proccesses.
     *
     * The UserActionManager class manages everything related to user actions
     * such as saving and recording actions. It is implemented as a
     * singleton pattern.
     */
    class UserActionManager : public QObject
    {
        Q_OBJECT
    public:

        /**
         * Get the singleton instance.
         *
         * @return The singleton instance.
         */
        static UserActionManager* instance();

        /**
         * Add executed action to history list.
         *
         * @param act - The action.
         */
        void addExecutedAction(UserAction* act);

        /**
         * Registers a new action factory.
         *
         * @param fac - The factory.
         */
        void registerFactory(UserActionFactory* fac);

        /**
         * Sets a start marker for macro recording.
         */
        void setStartRecording();

        /**
         * Stops macro recording and write commands to xml file.
         *
         * @param macroFilename - The file name.
         * @return Ok=stopped and saved  Discard=stopped not saved   Retry=try with other filename   Cancel=user changed his mind
         */
        QMessageBox::StandardButton setStopRecording(const QString& macroFilename);

        /**
         * Executes macro from file.
         *
         * @param macroFilename - The file name.
         */
        void playMacro(const QString& macroFilename);

        /**
         * Tests whether actions are currently recorded.
         *
         * @return True if it is currently recording, false otherwise.
         */
        bool isRecording() const;

        /**
         * Tests whether actions are currently recorded and
         * at least one command has been executed
         *
         * @return True if there are recorded and executed actions, False otherwise.
         */
        bool hasRecorded() const;

        /**
         * Creates UserAction instance for next command found by xml parser.
         *
         * @param xmlIn - The xml reader.
         *
         * @return The next UserAction.
         */
        UserAction* getParsedAction(QXmlStreamReader& xmlIn) const;

        /**
         * Get the elapsed time in milliseconds since GUI start.
         *
         * @return The elapsed time.
         */
        qint64 timeStamp() const { return mElapsedTime.elapsed(); }

        /**
         * Undo last action.
         */
        void undoLastAction();

        /**
         * Dumps actions from beginning.
         *
         * @param sig - Part of the macro file name.
         */
        void crashDump(int sig);

    private:
        UserActionManager(QObject *parent = nullptr);
        void testUndo();

        QList<UserAction*> mActionHistory;
        QHash<QString,UserActionFactory*> mActionFactory;
        int mStartRecording;
        QElapsedTimer mElapsedTime;
        bool mRecordHashAttribute;

        static UserActionManager* inst;
        QPlainTextEdit* mDumpAction;
        SettingsItemCheckbox* mSettingDumpAction;

    public Q_SLOTS:
        /**
         * Deletes current dumpAction if parameter is set to true.
         *
         * @param wantDump - Decides if current dumpAction is to be deleted.
         */
        void handleSettingDumpActionChanged(bool wantDump);

    Q_SIGNALS:
        /**
         * Q_SIGNAL that is emitted when undoLastAction() is called. The parameter is set to true
         * if the last action could be undone.
         *
         * @param yesWeCan - True if last action could be undone.
         */
        void canUndoLastAction(bool yesWeCan);
    };
}
