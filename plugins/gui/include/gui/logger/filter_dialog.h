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

#include "gui/logger/filter_item.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>

namespace hal
{
    class FilterTabBar;

    /**
     * @ingroup unused
     */
    class FilterDialog : public QDialog
    {
        Q_OBJECT

    public:
        FilterDialog(FilterTabBar* caller, QWidget* parent = nullptr);
        ~FilterDialog();

        void appendFilterItem(QString name, FilterItem* item);

    Q_SIGNALS:
        void inputValid();

    public Q_SLOTS:
        void verify();
        void reset(int);

    private:
        class FilterComboBox : public QComboBox
        {
        public:
            FilterComboBox(QWidget* parent = nullptr);

            FilterItem::Rule get_data();
        };

        FilterTabBar* mCaller;
        QVBoxLayout mContentLayout;
        QFormLayout mFormLayout;
        QGridLayout mGridLayout;

        QLineEdit mName;
        QLineEdit mKeywords;
        QLineEdit mRegex;

        QLabel mStatusMessage;

        FilterComboBox mTraceBox;
        FilterComboBox mDebugBox;
        FilterComboBox mInfoBox;
        FilterComboBox mWarningBox;
        FilterComboBox mErrorBox;
        FilterComboBox mCriticalBox;
        FilterComboBox mDefaultBox;

        QDialogButtonBox mButtonBox;
    };
}
