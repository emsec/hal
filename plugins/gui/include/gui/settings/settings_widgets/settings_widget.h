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
    class SettingsItem;

    class SettingsWidget : public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(QColor highlightColor READ highlightColor WRITE setHighlightColor)
        Q_PROPERTY(bool dirty READ dirty WRITE setDirty)

    public:
        explicit SettingsWidget(SettingsItem* item, QWidget* parent = 0);

        QColor highlightColor();
        void setHighlightColor(const QColor& color);

        void reset_labels();
        bool matchLabel(const QString& needle);

        bool dirty() const;
        void prepare();

        const SettingsItem* settingsItem() const { return mSettingsItem; }
        virtual void acceptValue();
        virtual void restoreDefault();
        virtual void loadCurrentValue();

        virtual void load(const QVariant& value) = 0;
        virtual QVariant value()                 = 0;

    public Q_SLOTS:
        void handleRevertModification();
        void handleSetDefaultValue();
        void handleItemDestroyed();

    Q_SIGNALS:
        void valueChanged();
        void triggerDescriptionUpdate(SettingsItem* item);
        void triggerRemoveWidget(SettingsWidget* widget);

    protected:
        void setDirty(bool dirty);
        void trigger_setting_updated();
        void enterEvent(QEvent* event);
        void leaveEvent(QEvent* event);

        QVBoxLayout* m_layout;
        QBoxLayout* mContainer;
        QHBoxLayout* m_top_bar;
        QLabel* mNameLabel;
        QToolButton* mRevertButton;
        QToolButton* mDefaultButton;

        QList<QPair<QLabel*, QString>> m_labels;
        SettingsItem* mSettingsItem;

    private:
        QColor mHighlightColor;
        bool mDirty = false;
    };
}
