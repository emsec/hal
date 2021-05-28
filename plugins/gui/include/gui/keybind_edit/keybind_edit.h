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

#include "gui/validator/stacked_validator.h"

#include <QKeySequenceEdit>
#include <QEvent>
#include <QColor>

namespace hal
{
    class SettingsItemKeybind;

    /**
     * @ingroup settings
     * @brief A widget to set or configure key shortcuts.
     *
     * The KeybindEdit class provides additional information such as an own validation
     * process that checks with hal's own shortcut management if a sequence already exists.
     */
    class KeybindEdit : public QKeySequenceEdit
    {
    Q_OBJECT
    Q_PROPERTY(bool hasGrab READ hasGrab WRITE setHasGrab)
    Q_PROPERTY(bool validated READ validated WRITE setValidated)

    public:
        /**
         * The constructor.
         *
         * @param parent - The
         */
        KeybindEdit(QWidget* parent = nullptr);

        /** @name Q_PROPERTY WRITE Functions
          */
        ///@{

        /**
         * Sets the hasGrab variable that should be set when the widget has the
         * keyboard focus (is set to true in the focusIn event and to false in
         * the focusOut event).
         *
         * @param isgrab - The value to set hasGrab state.
         */
        void setHasGrab(bool isgrab);

        /**
         * Sets the validation state.
         *
         * @param valid - The value to set.
         */
        void setValidated(bool valid);
        ///@}

        /** @name Q_PROPERTY READ Functions
          */
        ///@{

        /**
         * Get the hasGrab state.
         *
         * @return The hasGrab state.
         */
        bool hasGrab();

        /**
         * Get the validation state of the current sequence.
         *
         * @return The validation state.
         */
        bool validated();
        ///@}

        /**
         * Validates the current sequence by checking if no keysequence already exists
         * in the global keymap or the existing sequence belongs the settingsitem of this
         * widget.
         *
         * @return True if the sequence is valid. False otherwise.
         */
        bool doValidate();

        /**
         * Loads the given sequence and settingsitem by setting its internal state to the
         * parameters and then registering all information in the AssignedKeybindMap.
         *
         * @param seq - The keysequence to load.
         * @param item - The corresponding keybind item.
         */
        void load(const QKeySequence& seq, SettingsItemKeybind* item);

    Q_SIGNALS:

        /**
         * Q_SIGNAL that is emitted when the validation failed in doValidate().
         *
         * @param errmsg - The errormessage.
         */
        void editRejected(QString errmsg);

        /**
         * Q_SIGNAL that is emitted when the validation succeeded in doValidate().
         */
        void editAccepted();

    private Q_SLOTS:
        void restoreOldSequence();

    protected:
        bool event(QEvent* e) override;

    private:
        SettingsItemKeybind* mItem;
        QKeySequence mOldSequence;
        bool mSkipValidate;
        bool mValidated;
        bool mGrab;
    };
}
