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

    /**
     * @ingroup settings
     * @brief The base class for all specific SettingsWidget%s.
     *
     * The SettingsWidget class represents the visual part of a single setting.
     * It provides an interface and collection of commenly shared functions such as
     * load(), value() or matchLabel(). It displays and works on a SettingsItem
     * that represents the logical part.
     */
    class SettingsWidget : public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(bool dirty READ dirty WRITE setDirty)

    public:
        /**
         * The constructor.
         *
         * @param item - The underlying SettingsItem for the widget.
         * @param parent - The widget's parent.
         */
        explicit SettingsWidget(SettingsItem* item, QWidget* parent = nullptr);

        /**
         * Hides empty labels and shows non-empty labels.
         */
        void reset_labels();

        /**
         * Searches the given string in the label of this widget's underlying SettingsItem.
         *
         * @param needle - The string to search for.
         * @return True if the string was found. False otherwise.
         */
        bool matchLabel(const QString& needle);

        /** @name Q_PROPERTY READ Function
          */
        ///@{

        /**
         * Get the dirty state.
         *
         * @return The dirty state.
         */
        bool dirty() const;
        ///@}

        /**
         * Loads the value of its SettingsItem and adjusts the visuals accordingly
         * and sets its dirty state to false.
         */
        void prepare();

        /**
         * Get the SettingsItem that this widget represents.
         *
         * @return The SettingsItem.
         */
        const SettingsItem* settingsItem() const { return mSettingsItem; }

        /**
         * Sets the value of the SettingsItem to the in the visual part configured value.
         */
        virtual void acceptValue();

        /**
         * Loads the value of the SettingsItem and updates the visuals.
         */
        virtual void loadCurrentValue();

        /**
         * Updates the visual setting by the given value.
         *
         * @param value - The value to be loaded into the visual setting.
         */
        virtual void load(const QVariant& value) = 0;

        /**
         * Get the visual value of the setting.
         *
         * @return The visual value.
         */
        virtual QVariant value()                 = 0;

        /**
         * Clears the editor that is used to change the setting.
         */
        virtual void clearEditor() {;}

    public Q_SLOTS:

        /**
         * Loads the value of the SettingsItem and updates the visuals.
         * Does the same as loadCurrentValue().
         */
        virtual void handleRevertModification();

        /**
         * Loads the default value from the SettingsItem and updates the visuals.
         *
         * @param setAll - Not used.
         */
        virtual void handleSetDefaultValue(bool setAll=false);

        /**
         * Removes its SettingsItem pointer, emits the triggerRemoveWidget signal and closes itself.
         * Should be called when the underlying SettingsItem is destroyed or simply connected to the
         * item's destroyed signal.
         */
        void handleItemDestroyed();

    Q_SIGNALS:

        /**
         * Q_SIGNAL that is currently unused.
         */
        void valueChanged();

        /**
         * Q_SIGNAL that is emitted whenever the mouse enters or leaves the widget.
         *
         * @param item - The widget's SettingItem when entering or nullptr when leaving the widget.
         */
        void triggerDescriptionUpdate(SettingsItem* item);

        /**
         * Q_SIGNAL that is emitted when all cleanup is done and the widget is ready to be removed.
         * @param widget
         */
        void triggerRemoveWidget(SettingsWidget* widget);

    protected:

        /** @name Q_PROPERTY WRITE Function
          */
        ///@{

        /**
         * @brief Sets the dirty state.
         *
         * @param dirty - The value to set the state to.
         */
        void setDirty(bool dirty);
        ///@}

        /**
         * Compares its visual value to the SettingsItem's current value and default value and
         * sets the additional visual informations accordingly.
         */
        void trigger_setting_updated();

        /**
         * Overwritten Qt function. Please refer to Qt's documentation for additional information.
         *
         * @param event - The triggered event.
         */
        void enterEvent(QEvent* event);

        /**
         * Overwritten Qt function. Please refer to Qt's documentation for additional information.
         *
         * @param event - The triggered event.
         */
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
        bool mDirty = false;
    };
}
