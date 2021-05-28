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

class QHBoxLayout;
class QLabel;
class QPropertyAnimation;
class QVBoxLayout;
class QToolButton;

namespace hal
{
    /**
     * @ingroup gui
     * @brief One item in the welcome screen's RecentFiles section.
     *
     * One entry in the RecentFileWidget. It represents one recently used file and can be clicked to open the netlist
     * (i.e. parse hdl file or open .hal file) of the file.
     */
    class RecentFileItem : public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(bool hover READ hover)
        Q_PROPERTY(bool disabled READ disabled)
        Q_PROPERTY(QString iconPath READ iconPath WRITE setIconPath)
        Q_PROPERTY(QString iconStyle READ iconStyle WRITE setIconStyle)

    public:
        /**
         * Constructor.
         *
         * @param file - The file this RecentFileItem represents
         * @param parent - The parent widget
         */
        explicit RecentFileItem(const QString& file, QWidget* parent = nullptr);

        /**
         * Handles that the mouse enters the widget.
         *
         * @param event - The QEvent
         */
        void enterEvent(QEvent* event) override;

        /**
         * Handles that the mouse leaves the widget.
         *
         * @param event - The QEvent
         */
        void leaveEvent(QEvent* event) override;

        /**
         * Handles that the mouse was pressed within the widget.
         *
         * @param event
         */
        void mousePressEvent(QMouseEvent* event) override;

        /**
         * Gets the size hint of the stored widget.
         *
         * @returns the size hint of the widget
         */
        virtual QSize sizeHint() const override;

        /**
         * Captures the resize event so that if the file path does not fit in the widget anymore it is shortened.
         *
         * @param object - The object of the event
         * @param event -
         * @return
         */
        virtual bool eventFilter(QObject* object, QEvent* event) override;

        /**
         * The file this RecentFileItem represents.
         *
         * @returns the file
         */
        QString file() const;

        /**
         * (Re-)Initializes the appearance of the widget.
         */
        void repolish();

        /** @name Q_PROPERTY READ Functions
         */
        ///@{
        bool hover();
        bool disabled();
        QString iconPath();
        QString iconStyle();
        ///@}

        /** @name Q_PROPERTY WRITE Functions
         */
        ///@{
        void setHoverActive(bool active);
        void setDisabled(bool disable);
        void setIconPath(const QString& path);
        void setIconStyle(const QString& style);
        ///@}

    Q_SIGNALS:
        /**
         * Q_SIGNAL to notify that this item should be removed. Emitted after clicking the 'x'-button of the item.
         *
         * @param item - The item to remove.
         */
        void removeRequested(RecentFileItem* item);

    private:
        QWidget* mWidget;
        QHBoxLayout* mHorizontalLayout;
        QLabel* mIconLabel;
        QVBoxLayout* mVerticalLayout;
        QLabel* mNameLabel;
        QLabel* mPathLabel;
        QPropertyAnimation* mAnimation;
        QToolButton* mRemoveButton;

        QString mFile;
        QString mPath;

        bool mHover;
        bool mDisabled;

        QString mIconPath;
        QString mIconStyle;

        void handleCloseRequested();
    };
}
