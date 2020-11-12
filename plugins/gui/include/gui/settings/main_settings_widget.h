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

#include "gui/expanding_list/expanding_list_button.h"
#include <QList>
#include <QMap>
#include <QWidget>

class QFrame;
class QHBoxLayout;
class QPushButton;
class QScrollArea;
class QVBoxLayout;
class QScrollBar;

namespace hal
{
    class ExpandingListButton;
    class ExpandingListWidget;
    class Searchbar;
    class SettingsDisplay;
    class SettingsWidget;

    class MainSettingsWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit MainSettingsWidget(QWidget* parent = 0);
        bool handleAboutToClose();

    Q_SIGNALS:
        void close();

    public Q_SLOTS:
        void handleRestoreDefaultsClicked();
        void handleCancelClicked();
        void handleOkClicked();
        void handleButtonSelected(ExpandingListButton* button);
        void handleTextEdited(const QString& text);
        void handleSettingUpdated(SettingsWidget* sender, const QString& key, const QVariant& value);

    private:
        void initWidgets();
        void makeSection(const QString& label, const QString& name, const QString& iconPath);
        void registerWidget(const QString& section_name, SettingsWidget* widget);
        bool checkConflict(SettingsWidget* widget, const QVariant& value) const;
        void makeExclusiveGroup(const QString& name);
        void assignExclusiveGroup(const QString& group_name, SettingsWidget* widget);
        void releaseExclusiveGroup(const QString& group_name, SettingsWidget* widget);
        void hideAllSettings();
        void showAllSettings();
        void removeAllHighlights();
        bool saveSettings();
        void rollbackSettings();

        QHBoxLayout* mLayout;
        ExpandingListWidget* mExpandingListWidget;
        QVBoxLayout* mVerticalLayout;
        QScrollBar* mScrollbar;

        QFrame* mSearchbarContainer;
        QHBoxLayout* mSearchbarLayout;
        Searchbar* mSearchbar;

        QScrollArea* mScrollArea;
        QFrame* mContent;
        QHBoxLayout* mContentLayout;
        QFrame* mSettingsContainer;
        QVBoxLayout* mContainerLayout;

        QHBoxLayout* mButtonLayout;

        QPushButton* mRestoreDefaults;
        QPushButton* mCancel;
        QPushButton* mOk;

        QMap<QString, QList<SettingsWidget*>*> mMap;
        QMap<QString, ExpandingListButton*> mSections;
        QString mActiveSection;

        QList<SettingsWidget*> mAllSettings;

        QList<QString> mExclusiveGroups;
        QMap<SettingsWidget*, QString> mExclusiveW2g;
        QMap<QString, QList<SettingsWidget*>*> mExclusiveG2w;

        bool mResetToFirstElement;
    };
}
