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

    /**
     * @ingroup settings
     * @brief List that contains all SettingsWidget%s.
     *
     * A utility class used to store all SettingsWidget of the MainSettingsWidget.
     * It also provides an interface to partition the settings into multiple sections.
     */
    class MainSettingsList : public QList<SettingsWidget*>
    {
        QMap<QString,QList<SettingsWidget*> > mSectionMap;
    public:
        /**
          * The desctructor.
          */
        ~MainSettingsList();

        /**
         * Appends the widget to the list and adds them to the given section.
         *
         * @param sectionName - The section to which the widget belongs.
         * @param widget - The setting to add.
         */
        void registerWidget(const QString& sectionName, SettingsWidget* widget);

        /**
         * Removes the widget from all internal data structures and deletes it.
         *
         * @param widget - The widget to be removed.
         */
        void unregisterWidget(SettingsWidget* widget);

        /**
         * Get a list of all SettingsWidget%s belonging to the given section.
         *
         * @param s - The section's name.
         * @return The list of SettingsWidget%s.
         */
        QList<SettingsWidget*> section(const QString& s) const { return mSectionMap.value(s); }

        /**
         * Get a list of all underlying SettingsItem%s that are retrieved from the corresponding SettingWidget%s.
         *
         * @return The list of SettingsItem%s.
         */
        QList<const SettingsItem *> getItems() const;

        /**
         * Get all sections that do not contain any SettingsWidget%s.
         * @return The list of sections.
         */
        QStringList emptySections() const;

        /**
         * Calls the clearEditor() function of all SettingsWidget%s.
         */
        void clearAll();
    };

    /**
     * @brief The top-level widget that displays all settings.
     *
     * The MainSettingsWidget class displays all SettingWidget%s and sections.
     * It manages the top-level functionality such as filtering for specific
     * widgets or restoring all default values.
     */
    class MainSettingsWidget : public QWidget
    {
        Q_OBJECT

    public:
        /**
         * The constructor.
         *
         * @param parent - The widget's parent.
         */
        explicit MainSettingsWidget(QWidget* parent = nullptr);

        /**
         * Checks all SettingWidget%s if they are in an unsaved modified state.
         *
         * @return True if any SettingWidget is in a modified state. False otherwise.
         */
        bool handleAboutToClose();

        /**
         * Initializes all SettingsWidget%s and loads its values.
         */
        void activate();

        /**
         * Unselects all sections and displays all settings.
         */
        void showAllSettings();

    Q_SIGNALS:
        /**
         * Q_SIGNAL that is emitted when either the Ok or Cancel button is clicked.
         */
        void close();

    public Q_SLOTS:

        /**
         * Restores the default value of each SettingsWidget.
         */
        void handleRestoreDefaultsClicked();

        /**
         * Emits close() and closes the widget.
         */
        void handleCancelClicked();

        /**
         * Saves all settings, emits close() and closes the widget.
         */
        void handleOkClicked();

        /**
         * Hides all widgets except the ones belonging to the section that corresponds
         * to the given button.
         *
         * @param button - The button that represents the section.
         */
        void handleButtonSelected(ExpandingListButton* button);

        /**
         * Updates the settings description if it is currently displayed at the bottom
         * of this widget.
         *
         * @param activeSettingsItem - The SettingsItem whose description has changed.
         */
        void handleDescriptionUpdate(SettingsItem* activeSettingsItem);

        /**
         * Removes the given widget from the layout and unregisters it from its MainSettingsList.
         *
         * @param widget - The widget to remove.
         */
        void handleWidgetRemove(SettingsWidget* widget);

        /**
         * Filters its SettingsWidget%s by the given string.
         *
         * @param needle - The string to filter by.
         */
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
