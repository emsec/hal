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

#include "settings/preview_widget.h"

#include <QVariant>
#include <QToolButton>
#include <QBoxLayout>
#include <QFrame>
#include <QPair>

class QColor;
class QLabel;
class QRegularExpression;
class QVBoxLayout;

namespace hal
{
    class SettingsWidget : public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(QColor highlight_color READ highlight_color WRITE set_highlight_color)
        Q_PROPERTY(bool dirty READ dirty WRITE set_dirty)
        Q_PROPERTY(bool conflicts READ conflicts WRITE set_conflicts)

    public:
        enum class preview_position
        {
            bottom = 0,
            right = 1
        };

        explicit SettingsWidget(const QString& key, QWidget* parent = 0);

        QColor highlight_color();
        QString key();
        void set_highlight_color(const QColor& color);

        void reset_labels();
        bool match_labels(const QString& string);

        void trigger_setting_updated();
        void set_dirty(bool dirty);
        bool dirty() const;
        void prepare(const QVariant& value, const QVariant& default_value);
        void mark_saved();
        void set_conflicts(bool conflicts);
        bool conflicts() const;

        void set_preview_widget(PreviewWidget* widget);
        void set_preview_position(preview_position position);

        virtual void load(const QVariant& value) = 0;
        virtual QVariant value()                  = 0;

    public Q_SLOTS:
        void handle_rollback();
        void handle_reset();

    Q_SIGNALS:
        void setting_updated(SettingsWidget* sender, const QString& key, const QVariant& value);

    protected:
        QVBoxLayout* m_layout;
        QBoxLayout* m_container;
        QHBoxLayout* m_top_bar;
        QLabel* m_name;
        QToolButton* m_revert;
        QToolButton* m_default;

        QList<QPair<QLabel*, QString>> m_labels;

    private:
        QColor m_highlight_color;
        QString m_key;
        bool m_signals_enabled = true;
        bool m_prepared = false;
        bool m_dirty = false;
        bool m_conflicts = false;
        QVariant m_loaded_value;
        QVariant m_default_value;
        PreviewWidget* m_preview = nullptr;
    };
}
