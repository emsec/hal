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

#include "gui/gui_def.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>

namespace hal
{
    /**
     * @ingroup file_management
     * @brief A dialog in form of a bar to let the user decide how to handle file changes outside of HAL.
     *
     * The FileModifiedBar class is a utility class for every widget that (might) interacts with
     * files on the disk. It provides a basic window with 3 situation dependent buttons
     * (Ok, Reload, Ignore). This class should be connected to a file watcher object so that
     * this classes handleFileChanged method can be correctly invoked.
     */
    class FileModifiedBar : public QFrame
    {
        Q_OBJECT

    public:

        /**
         * The constructor.
         *
         * @param parent - The widget's parent.
         */
        FileModifiedBar(QWidget* parent = nullptr);

        /**
         * Not implemented. Probably intented to show custom messages when the underlying
         * file is modified, moved, or deleted.
         *
         * @param message - The message to show (probably).
         */
        void setMessage(QString message);

    Q_SIGNALS:

        /**
         * Q_SIGNAL that is emitted when the reload button is clicked.
         */
        void reloadClicked();

        /**
         * Q_SIGNAL that is emitted when the ignore button is clicked.
         */
        void ignoreClicked();

        /**
         * Q_SIGNAL that is emitted when the ok button is clicked.
         */
        void okClicked();

    public Q_SLOTS:

        /**
         * Checks if the file that is specified with the given path exists and enables/disables
         * its buttons accordingly. This function should be connected to a file watcher object.
         *
         * @param path - The path of the file that was either modified, moved, or deleted.
         */
        void handleFileChanged(QString path);

    private:
        QHBoxLayout* mLayout;
        QLabel* mMessageLabel;
        QPushButton* mReloadButton;
        QPushButton* mIgnoreButton;
        QPushButton* mOkButton;

        void handleReloadClicked();
        void handleIgnoreClicked();
        void handleOkClicked();

        void handleFileContentModified(QString path);
        void handleFileMovOrDel(QString path);
    };
}
