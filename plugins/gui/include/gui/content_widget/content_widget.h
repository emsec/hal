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

#include "gui/widget/widget.h"

#include <QIcon>
#include <QList>

class QShortcut;
class QVBoxLayout;

namespace hal
{
    class ContentAnchor;
    class Toolbar;

    /**
     * @ingroup gui
     * @brief Abstract class for Widget%s within HAL's ContentArea
     *
     * The ContentWidget class is the base class that has to be derived from in order to be put in hal's ContentArea. It
     * provides the necessary interface for the docking system as well as functions that can be overwritten to configure its
     * shortcuts and toolbar.
     */
    class ContentWidget : public Widget
    {
        Q_OBJECT
        Q_PROPERTY(QString iconStyle READ iconStyle WRITE setIconStyle)
        Q_PROPERTY(QString iconPath READ iconPath WRITE setIconPath)

    public:
        /**
         * The constructor.
         *
         * @param name - The in the top left corner displayed name.
         * @param parent - The widget's parent.
         */
        explicit ContentWidget(QString name, QWidget* parent = nullptr);

        /**
         * A virtual function that can be overwritten by the actual widget to configure its toolbar (add actions to it and connect them).
         * It is called by the wrapper class ContentFrame that actually displays the toolbar.
         *
         * @param Toolbar - The toolbar to add actions to.
         */
        virtual void setupToolbar(Toolbar* Toolbar);

        /**
         * A virtual function that can be overwritten by the actual widget to create the widget's keyboard shortcuts.
         * Shortcuts are created and managed with the help of the keybind manager.
         *
         * @return The List of (created) QShortcuts.
         */
        virtual QList<QShortcut*> createShortcuts();

        /**
         * Reapplies the current Stylesheet by using the QStyle's unpolish and polish functions. It is necessary because
         * certain aspects of a widgets are not automatically set and rendered just by calling setStylesheet() when
         * switched to a new one (for example when a new theme wants to be applied at runtime).
         */
        void repolish();

        /**
         * Get the widget's displayed name (actually displayed within the ContentFrame's area).
         *
         * @return The name.
         */
        QString name();

        /**
         * Get the loaded and processed icon that can be set in the stylesheet.
         *
         * @return The icon.
         */
        QIcon icon();

        /**
         * Sets the anchor of the content widget. This function is used by the docking system.
         *
         * @param anchor - The anchor.
         */
        void setAnchor(ContentAnchor* anchor);

        /**
         * Sets the icon of the widget.
         *
         * @param icon - The icon.
         */
        void setIcon(QIcon icon);

        /**
         * Sets the displayed name of the widget. Usefull when the name changes during runtime (e.g. when the content widget
         * consists of multiple layered sub widget and each has its own "subname").
         *
         * @param name - The new name.
         */
        void set_name(const QString& name);

        /** @name Q_PROPERTY READ Functions
        */
        ///@{

        /**
         * Necessary function used by qt's property system. Refer to Qt's documentation to learn more.
         *
         * @return The style descrition needed when processing the icon.
         */
        QString iconStyle();

        /**
         * Necessary function used by qt's property system. Refer to Qt's documentation to learn more.
         *
         * @return The icon's path.
         */
        QString iconPath();
        ///@}

        /** @name Q_PROPERTY WRITE Functions
          */
        ///@{

        /**
         * Necessary function used by qt's property system. Refer to Qt's documentation to learn more.
         *
         * @param style - The style description of the icon.
         */
        void setIconStyle(const QString& style);

        /**
         * Necessary function used by qt's property system. Refer to Qt's documentation to learn more.
         *
         * @param path - The icon's path.
         */
        void setIconPath(const QString& path);
        ///@}

    Q_SIGNALS:

        /**
         * Q_SIGNAL that is emitted when the widget is removed from its anchor (related to the docking system).
         */
        void removed();

        /**
         * Q_SIGNAL that is emitted when the widget is detached from its anchor (related to the docking system).
         */
        void detached();

        /**
         * Q_SIGNAL that is emitted when the widget is reattached to its anchor (related to the docking system).
         */
        void reattached();

        /**
         * Q_SIGNAL that is emitted when the widget is opened (displayed in the anchor) (related to the docking system).
         */
        void opened();

        /**
         * Q_SIGNAL that is emitted when the widget is closed (hidden in the anchor) (related to the docking system).
         */
        void closed();

        /**
         * Q_SIGNAL that is emitted when the widget's name is changed after calling setName().
         *
         * @param name - The new name.
         */
        void name_changed(const QString& name);

    public Q_SLOTS:

        /**
         * Removes the widget from its anchor (related to the docking system).
         */
        void remove();

        /**
         * Detaches the widget from its anchor (related to the docking system).
         */
        void detach();

        /**
         * Reattaches the widget to its anchor (related to the docking system).
         */
        void reattach();

        /**
         * Opens (displays) the widget in its anchor (related to the docking system).
         */
        void open();

        /**
         * Closes (hides) the widget in its anchor (related to the docking system).
         */
        void close();

        /**
         * Q_SLOT to handle that the settings for the search key sequence (default: 'Ctrl+F') has been changed.
         * Configures the new search key sequence for the ContentWidget.
         *
         * @param seq - The new search key sequence
         */
        void handleSearchKeysequenceChanged(QKeySequence seq);

    private:
        void closeEvent(QCloseEvent* event);

        QString mName;
        QIcon mIcon;
        ContentAnchor* mAnchor = nullptr;
        int mIndexPriority = 0;

        QString mIconStyle;
        QString mIconPath;

    protected:
        QVBoxLayout* mContentLayout;
        QAction* mSearchAction;
        QShortcut* mSearchShortcut;
        QKeySequence mSearchKeysequence;
    };
}
