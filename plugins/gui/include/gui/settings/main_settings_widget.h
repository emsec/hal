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
class QTextEdit;

namespace hal
{
    class ExpandingListButton;
    class ExpandingListWidget;
    class SettingsDisplay;
    class SettingsWidget;
    class SettingsItem;
    class Searchbar;

    class MainSettingsList : public QList<SettingsWidget*>
    {
        QMap<QString,QList<SettingsWidget*> > mSectionMap;
    public:
        ~MainSettingsList();
        void registerWidget(const QString& sectionName, SettingsWidget* widget);
        void unregisterWidget(SettingsWidget* widget);
        QList<SettingsWidget*> section(const QString& s) const { return mSectionMap.value(s); }
        QList<const SettingsItem *> getItems() const;
        QStringList emptySections() const;
        void clearAll();
    };

    class MainSettingsWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit MainSettingsWidget(QWidget* parent = nullptr);
        bool handleAboutToClose();
        void activate();
        void showAllSettings();

    Q_SIGNALS:
        void close();

    public Q_SLOTS:
        void handleRestoreDefaultsClicked();
        void handleCancelClicked();
        void handleOkClicked();
        void handleButtonSelected(ExpandingListButton* button);
        void handleDescriptionUpdate(SettingsItem* activeSettingsItem);
        void handleWidgetRemove(SettingsWidget* widget);
        void searchSettings(const QString& needle);

    private:
        void initWidgets();
        void makeSection(const QString& label);
        void hideAllSettings();
        bool saveSettings();

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
        QTextEdit* mDescriptionText;

        QPushButton* mRestoreDefaults;
        QPushButton* mCancel;
        QPushButton* mOk;

        QMap<ExpandingListButton*,QString> mSectionNames;
        QString mActiveSection;

        MainSettingsList mSettingsList;

        bool mResetToFirstElement;
    };
}
