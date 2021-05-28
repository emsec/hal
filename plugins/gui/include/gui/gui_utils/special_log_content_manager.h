//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
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

//forward declaration
class QTimer;
class QTabWidget;

namespace hal
{
    class MainWindow;
    class PythonEditor;
    class PythonCodeEditor;

    /**
     * @ingroup gui
     * @brief Logs the python editor and gui screenshots.
     *
     * The SpecialLogContentManager class creates special log content that is used for meta analysis.
     * It takes screenshots of the MainWindow (and all detached widgets) as well as dumps every python editor tabs
     * in a certain interval. The logged content is saved in a directory that is located next to the opened file.
     */
    class SpecialLogContentManager : public QObject
    {
        Q_OBJECT
    public:
        /**
         * The constructor.
         * @param parent - The manager's parent.
         * @param python_editor - The editor which tabs are to be logged (dumped).
         */
        SpecialLogContentManager(QObject* parent = nullptr, PythonEditor* python_editor = nullptr);

        /**
         * The default destructor.
         */
        ~SpecialLogContentManager();

        /**
         * Creates (if it does not exist already) a directory named after the currently opened file.
         * It takes a screenshot consisting of every top-level-widget of hal and saves it
         * to the created directory in an interval that is specified when calling startLogging(int msec).
         */
        void safeScreenshot();

        /**
         * Creates (if it does not exist already) a directory named after the currently opened file.
         * It takes the content of every python editor tab, combines them into a single file
         * and saves it to the created directory in an interval that is specified when calling startLogging(int msec).
         */
        void safePythonEditor();

        /**
         * Starts the logging process of the manager by specifying an interval in which to log the content.
         *
         * @param msec - The interval in milliseconds.
         */
        void startLogging(int msec);

    private:
        QTimer* mTimer;
        PythonEditor* mPythonEditor;
        //either save it in the same directory (eg. /SpecialLogs) or split it into something
        //like (SpecialLogs/Screenshots and SpecialLogs/PyhthonContent)
        QString mScreenshotSubPath;
        QString mPythonContentSubPath;
    };
}
