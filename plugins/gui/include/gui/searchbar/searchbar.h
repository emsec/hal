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

#include <QFrame>

class QLabel;
class QLineEdit;
class QHBoxLayout;
class QPushButton;
class QToolButton;

namespace hal
{
    /**
     * @ingroup gui
     * @brief A QFrame with a QLineEdit that can be used to input a substring to search for
     *
     * The Searchbar consists of a QLineEdit which can be filled with a substring to search for. If the text within
     * the searchbar changes, a signal textEdited is emitted. Other objects that use the input of the Searchbar may
     * want to connect to this signal to update their respective view.
     *
     * The searchbar also contains a 'Mode'-button. When clicked the signal modeClicked is emitted.
     */
    class Searchbar : public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(QString searchIcon READ searchIcon WRITE setSearchIcon)
        Q_PROPERTY(QString searchIconStyle READ searchIconStyle WRITE setSearchIconStyle)
        Q_PROPERTY(QString clearIcon READ clearIcon WRITE setClearIcon)
        Q_PROPERTY(QString clearIconStyle READ clearIconStyle WRITE setClearIconStyle)

    public:
        /**
         * Constructor.
         *
         * @param parent - The parent widget
         */
        Searchbar(QWidget* parent = nullptr);

        /// @name Q_PROPERTY READ Functions
        ///@{
        QString searchIcon() const;
        QString searchIconStyle() const;
        QString clearIcon() const;
        QString clearIconStyle() const;
        ///@}

        /// @name Q_PROPERTY WRITE Functions
        ///@{
        void setSearchIcon(const QString& icon);
        void setSearchIconStyle(const QString& style);
        void setClearIcon(const QString& icon);
        void setClearIconStyle(const QString& style);
        ///@}

        /**
         * Sets the placeholder text of the QLineEdit of this Searchbar. The placeholder text is displayed if the
         * QLineEdit is empty.
         *
         * @param text - The new placeholder text
         */
        void setPlaceholderText(const QString& text);

        /**
         * Sets the label text of the 'Mode'-button.
         *
         * @param text - The new 'Mode'-button text
         */
        void setModeButtonText(const QString& text);

        /**
         * Empties the Searchbar's QLineEdit.
         */
        void clear();

        /**
         * Gets the current Searchbar input, i.e. the current text in the Searchbar's QLineEdit.
         *
         * @return the current search string
         */
        QString getCurrentText();

        /**
         * Reinitializes the appearance of the Searchbar.
         */
        void repolish();

    Q_SIGNALS:
        /**
         * Q_SIGNAL that is emitted whenever the search string (i.e. the string withing the Searchbar's QLineEdit)
         * has been changed.
         *
         * @param text - The new search string
         */
        void textEdited(const QString& text);

        /**
         * Q_SIGNAL that is emitted whenever the Return/Enter key has been pressed.
         */
        void returnPressed();

        /**
         * Q_SIGNAL that is emitted whenever the 'Mode'-Button has been clicked.
         */
        void modeClicked();

    public Q_SLOTS:
        /**
         * Handles changes within the QLineEdit. Emits the signal textEdited.
         *
         * @param text - The new search string
         */
        void handleTextEdited(const QString& text);

        /**
         * Handles Return/Enter key pressed. Emits the signal returnPressed.
         */
        void handleReturnPressed();

        /**
         * Handles 'Mode'-Button clicks. Emits the signal modeClicked.
         */
        void handleModeClicked();

    private:
        QHBoxLayout* mLayout;

        QLabel* mSearchIconLabel;
        QLineEdit* mLineEdit;
        QLabel* mClearIconLabel;
        QPushButton* mModeButton;
        QToolButton* mDownButton;
        QToolButton* mUpButton;

        QString mSearchIcon;
        QString mSearchIconStyle;
        QString mClearIcon;
        QString mClearIconStyle;
    };
}
