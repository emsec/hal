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

#include "gui/gui_def.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>

namespace hal
{
    class FileModifiedBar : public QFrame
    {
        Q_OBJECT

    public:
        FileModifiedBar(QWidget* parent = 0);

        void setMessage(QString message);

    Q_SIGNALS:
        void reloadClicked();
        void ignoreClicked();
        void okClicked();

    public Q_SLOTS:
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
