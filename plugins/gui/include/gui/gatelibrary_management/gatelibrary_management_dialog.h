// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <QDialog>
#include <QLabel>
#include <QLayout>
#include <QModelIndex>
#include <QTableView>
#include <QPushButton>

namespace hal
{

class GatelibraryProxyModel;
class GatelibraryTableModel;
class GatelibraryTableView;

class Searchbar;

    /**
     * @ingroup gui
     * @brief A dialog for the management of gate libraries
     *
     * The AboutDialog is a QDialog that can be opened via File->Gate Libraries.
     * It can be used to load, unload and reload gate libraries.
     */
    class GatelibraryManagementDialog : public QDialog
    {
        Q_OBJECT
        Q_PROPERTY(QString loadIconPath READ loadIconPath WRITE setLoadIconPath)
        Q_PROPERTY(QString loadIconStyle READ loadIconStyle WRITE setLoadIconStyle)

        Q_PROPERTY(QString reloadIconPath READ reloadIconPath WRITE setReloadIconPath)
        Q_PROPERTY(QString reloadIconStyle READ reloadIconStyle WRITE setReloadIconStyle)

        Q_PROPERTY(QString removeIconPath READ removeIconPath WRITE setRemoveIconPath)
        Q_PROPERTY(QString removeIconStyle READ removeIconStyle WRITE setRemoveIconStyle)

    public:
        /**
         * Constructor. Initializes the Dialog.
         *
         * @param parent - The parent widget.
         */
        explicit GatelibraryManagementDialog(QWidget* parent = nullptr);

        /**
         * (Re-)initializes the appearance of the widget.
         */
        void repolish();

        /** @name Q_PROPERTY READ Functions
         */
        ///@{
        QString loadIconPath();
        QString loadIconStyle();

        QString reloadIconPath();
        QString reloadIconStyle();

        QString removeIconPath();
        QString removeIconStyle();
        ///@}

        /** @name Q_PROPERTY WRITE Functions
         */
        ///@{
        void setLoadIconPath(const QString& path);
        void setLoadIconStyle(const QString& style);

        void setReloadIconPath(const QString& path);
        void setReloadIconStyle(const QString& style);

        void setRemoveIconPath(const QString& path);
        void setRemoveIconStyle(const QString& style);
        ///@}

    private Q_SLOTS:
        void handleLoadButtonClicked();
        void handleReloadButtonClicked();
        void handleRemoveButtonClicked();

        void handleCurrentRowChanged(const QModelIndex& current, const QModelIndex& previous);

        void filter(const QString& text);

    private:
        QString mLoadIconPath;
        QString mLoadIconStyle;

        QString mReloadIconPath;
        QString mReloadIconStyle;

        QString mRemoveIconPath;
        QString mRemoveIconStyle;

        QVBoxLayout* mLayout;
        Searchbar* mSearchbar;
        GatelibraryTableModel* mTableModel;
        GatelibraryProxyModel* mProxyModel;
        GatelibraryTableView* mTableView;
        QHBoxLayout* mHLayout;
        QPushButton* mLoadButton;
        QPushButton* mRemoveButton;
        QPushButton* mReloadButton;
    };
}

