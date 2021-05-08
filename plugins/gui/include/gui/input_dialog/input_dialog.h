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

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace hal
{

    /**
     * @ingroup gui
     * @brief Generic input dialog for user input.
     *
     * The InputDialog is a dialog that requires an input from the user.
     *
     * The return value of the exec-function is QDialog::Accepted if the user clicked 'Ok'. It is QDialog::Rejected if
     * the user clicked 'Cancel'.
     *
     * After the execution the user entry can be extracted by calling the function 'textValue'.
     */
    class InputDialog : public QDialog
    {
        public:
            /**
             * Empty constructor.
             *
             * @param parent - The parent widget
             * @param f - The WindowFlags that are passed to the QWidget constructor
             */
            InputDialog(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

            /**
             * Constructor.
             *
             * @param window_title - The title of the window
             * @param info_text - The description of the entry the user has to make
             * @param input_text - The entry that is set in the input field by default
             * @param parent - The parent widget
             * @param f - The WindowFlags that are passed to the QWidget constructor
             */
            InputDialog(const QString& window_title, const QString& info_text, const QString& input_text, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

            /**
             * Sets the description of the entry the user has to make that is displayed above the input field.
             *
             * @param text - The new info text
             */
            void setInfoText(const QString& text);

            /**
             * Sets the default text that is entered in the input filed at the beginning.
             *
             * @param text - The new input text
             */
            void setInputText(const QString& text);

            /**
             * Sets the warning that is displayed below the input field if the current input is invalid.
             *
             * @param text - The warning text
             */
            void setWarningText(const QString& text);

            /**
             * Adds a validator that checks for the current input whether it is valid or not.
             *
             * @param Validator - The validator to add
             */
            void addValidator(Validator* Validator);

            /**
             * Removes a validator.
             *
             * @param Validator - The validator to remove.
             */
            void removeValidator(Validator* Validator);

            /**
             * Removes all current validators.
             */
            void clearValidators();

            /**
             * Gets the current content of the input field. Can by used after the execution to extract the
             * users input.
             *
             * @return the current content of the input field.
             */
            QString textValue() const;

        private:
            QVBoxLayout* mLayout;
            QLabel* mLabelInfoText;
            QLabel* mLabelWarningText;
            QLineEdit* mInputTextEdit;
            QPushButton* mOkButton;

            QString mWarningText = "";

            StackedValidator mValidator;

            void init();

            void showWarningText();
            void hideWarningText();

            void enableOkButton();
            void disableOkButton();

            void enableProgression();
            void disableProgression();

            void handleOkClicked();
            void handleCancelClicked();

            void handleTextChanged(const QString &text);
    };
}
