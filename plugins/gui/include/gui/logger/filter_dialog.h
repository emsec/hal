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

#include "logger/filter_item.h"
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

    class FilterDialog : public QDialog
    {
        Q_OBJECT

    public:
        FilterDialog(FilterTabBar* caller, QWidget* parent = 0);

        ~FilterDialog();

        void append_filter_item(QString name, FilterItem* item);

    Q_SIGNALS:

        void input_valid();

    public Q_SLOTS:

        void verify();

        void reset(int);

    private:
        class filter_combo_box : public QComboBox
        {
        public:
            filter_combo_box(QWidget* parent = 0);

            FilterItem::rule get_data();
        };

        FilterTabBar* m_caller;

        QVBoxLayout m_content_layout;

        QFormLayout m_form_layout;

        QGridLayout m_grid_layout;

        QLineEdit m_name;

        QLineEdit m_keywords;

        QLineEdit m_regex;

        QLabel m_status_message;

        filter_combo_box m_trace_box;

        filter_combo_box m_debug_box;

        filter_combo_box m_info_box;

        filter_combo_box m_warning_box;

        filter_combo_box m_error_box;

        filter_combo_box m_critical_box;

        filter_combo_box m_default_box;

        QDialogButtonBox m_button_box;
    };
}
