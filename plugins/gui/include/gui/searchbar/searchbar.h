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
     */
    class Searchbar : public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(QString searchIcon READ searchIcon WRITE setSearchIcon)
        Q_PROPERTY(QString searchIconStyle READ searchIconStyle WRITE setSearchIconStyle)
        Q_PROPERTY(QString clearIcon READ clearIcon WRITE setClearIcon)
        Q_PROPERTY(QString clearIconStyle READ clearIconStyle WRITE setClearIconStyle)
        Q_PROPERTY(QString caseSensitivityIcon READ caseSensitivityIcon WRITE setCaseSensitivityIcon)
        Q_PROPERTY(QString caseSensitivityIconStyle READ caseSensitivityIconStyle WRITE setCaseSensitivityIconStyle)
        Q_PROPERTY(QString exactMatchIcon READ exactMatchIcon WRITE setExactMatchIcon)
        Q_PROPERTY(QString exactMatchIconStyle READ exactMatchIconStyle WRITE setExactMatchIconStyle)

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
        QString caseSensitivityIcon() const;
        QString caseSensitivityIconStyle() const;
        QString exactMatchIcon() const;
        QString exactMatchIconStyle() const;
        ///@}

        /// @name Q_PROPERTY WRITE Functions
        ///@{
        void setSearchIcon(const QString& icon);
        void setSearchIconStyle(const QString& style);
        void setClearIcon(const QString& icon);
        void setClearIconStyle(const QString& style);
        void setCaseSensitivityIcon(const QString& icon);
        void setCaseSensitivityIconStyle(const QString& style);
        void setExactMatchIcon(const QString& icon);
        void setExactMatchIconStyle(const QString& style);
        ///@}

        /**
         * Sets the placeholder text of the QLineEdit of this Searchbar. The placeholder text is displayed if the
         * QLineEdit is empty.
         *
         * @param text - The new placeholder text
         */
        void setPlaceholderText(const QString& text);

        /**
         * Empties the Searchbar's QLineEdit.
         */
        void clear();

        /**
         * Emits textEdited with empty string when hide() is called
         */
        void hideEvent(QHideEvent *) override;

        /**
         * Emits textEdited with getCurrentText() when show() is called
         */
        void showEvent(QShowEvent *) override;

        /**
         * Gets the current Searchbar input, i.e. the current text in the Searchbar's QLineEdit.
         *
         * @return the current search string
         */
        QString getCurrentText();

        /**
         * Gets the current Searchbar input with regex modifier.
         *
         * @return the current search string
         */
        QString getCurrentTextWithFlags();

        /**
         * Adds flags ("Exact Match" or "Case Sensitive") as regex modifiers to text.
         *
         * @return text with regex modifiers as QString
         */
        QString addFlags(const QString& text);

        /**
         * Reinitializes the appearance of the Searchbar.
         */
        void repolish();

        /**
         * @return mExactMatch->isChecked()
         */
        bool exactMatchChecked();

        /**
         * @return mCaseSensitive->isChecked()
         */
        bool caseSensitiveChecked();

        /**
         * A filter is applied when the searchbar contains text or (at least) one of the flags
         * ("Exact Match" or "Case Sensitive") is checked.
         *
         * @return if a filter is applied
         */
        bool filterApplied();

        void setEmitTextWithFlags(bool);

        bool getEmitTextWithFlags();

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

    public Q_SLOTS:
        /**
         * Emits textEdited with respect to mEmitTextWithFlags.
         */
        void emitTextEdited();

        /**
         * Handles Return/Enter key pressed. Emits the signal returnPressed.
         */
        void handleReturnPressed();

        /**
         * Handles "Clear"-Button clicks.
         */
        void handleClearClicked();

    private:
        QHBoxLayout* mLayout;

        QLabel* mSearchIconLabel;
        QLineEdit* mLineEdit;
        QLabel* mClearIconLabel;

        QToolButton* mDownButton;
        QToolButton* mUpButton;
        QToolButton* mCaseSensitiveButton;
        QToolButton* mExactMatchButton;
        QToolButton* mClearButton;

        QString mSearchIcon;
        QString mSearchIconStyle;
        QString mClearIcon;
        QString mClearIconStyle;
        QString mCaseSensitivityIcon;
        QString mCaseSensitivityIconStyle;
        QString mExactMatchIcon;
        QString mExactMatchIconStyle;

        // One can decide wether to receive the text (emitted by textEdited) with or without regex modifier
        // If set to false, one has to manually implement 'Exact Match'/'Case Sensitive' functionality
        bool mEmitTextWithFlags = true;
    };
}
