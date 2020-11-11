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

#include "gui/settings/preview_widget.h"

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
        Q_PROPERTY(QColor highlightColor READ highlightColor WRITE setHighlightColor)
        Q_PROPERTY(bool dirty READ dirty WRITE setDirty)
        Q_PROPERTY(bool conflicts READ conflicts WRITE setConflicts)

    public:
        enum class preview_position
        {
            bottom = 0,
            right = 1
        };

        explicit SettingsWidget(const QString& key, QWidget* parent = 0);

        QColor highlightColor();
        QString key();
        void setHighlightColor(const QColor& color);

        void resetLabels();
        bool matchLabels(const QString& string);

        bool dirty() const;
        void prepare(const QVariant& value, const QVariant& default_value);
        void markSaved();
        void setConflicts(bool conflicts);
        bool conflicts() const;

        void setPreviewWidget(PreviewWidget* widget);
        void setPreviewPosition(preview_position position);

        virtual void load(const QVariant& value) = 0;
        virtual QVariant value()                  = 0;

    public Q_SLOTS:
        void handleRollback();
        void handleReset();

    Q_SIGNALS:
        void settingUpdated(SettingsWidget* sender, const QString& key, const QVariant& value);

    protected:
        void setDirty(bool dirty);
        void triggerSettingUpdated();
        void enterEvent(QEvent* event);
        void leaveEvent(QEvent* event);

        QVBoxLayout* mLayout;
        QBoxLayout* mContainer;
        QHBoxLayout* mTopBar;
        QLabel* mName;
        QToolButton* mRevert;
        QToolButton* mDefault;

        QList<QPair<QLabel*, QString>> mLabels;

    private:
        QColor mHighlightColor;
        QString mKey;
        bool mSignalsEnabled = true;
        bool mPrepared = false;
        bool mDirty = false;
        bool mConflicts = false;
        QVariant mLoadedValue;
        QVariant mDefaultValue;
        PreviewWidget* m_preview = nullptr;
    };
}
