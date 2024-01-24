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

#include "hal_core/defines.h"

#include <QAction>
#include <QFrame>
#include <QTableView>
#include <gui/pin_model/pin_proxy_model.h>

namespace hal
{
    class GatelibraryTableModel;
    class GatelibraryManager;
    class Toolbar;
    class Searchbar;

    class GatelibraryContentWidget : public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(QString disabledIconStyle READ disabledIconStyle WRITE setDisabledIconStyle)
        Q_PROPERTY(QString enabledIconStyle READ enabledIconStyle WRITE setEnabledIconStyle)
        Q_PROPERTY(QString addTypeIconPath READ addTypeIconPath WRITE setAddTypeIconPath)
        Q_PROPERTY(QString editTypeIconPath READ editTypeIconPath WRITE setEditTypeIconPath)
        Q_PROPERTY(QString searchIconPath READ searchIconPath WRITE setSearchIconPath)
        Q_PROPERTY(QString searchIconStyle READ searchIconStyle WRITE setSearchIconStyle)
        Q_PROPERTY(QString searchActiveIconStyle READ searchActiveIconStyle WRITE setSearchActiveIconStyle)
        Q_PROPERTY(QString deleteIconPath READ deleteIconPath WRITE setDeleteIconPath);
        Q_PROPERTY(QString deleteIconStyle READ deleteIconStyle WRITE setDeleteIconStyle);

        friend class GateLibraryManager;

        QTableView* mTableView;
        PinProxyModel* mPinProxyModel;
        Searchbar* mSearchbar;
        Toolbar* mToolbar;
        QAction* mAddAction;
        QAction* mEditAction;
        QAction* mSearchAction;
        QAction* mDeleteAction;

        QString mDisabledIconStyle;
        QString mEnabledIconStyle;
        QString mAddTypeIconPath;
        QString mEditTypeIconPath;
        QString mSearchIconPath;
        QString mSearchIconStyle;
        QString mSearchActiveIconStyle;
        QString mDeleteIconPath;
        QString mDeleteIconStyle;

        bool mReadOnly = false;

    Q_SIGNALS:
        void triggerEditType(QModelIndex index);
        void triggerDeleteType(QModelIndex index);
        void triggerCurrentSelectionChanged(QModelIndex index, QModelIndex prevIndex);
        void triggerDoubleClicked(QModelIndex index);

    private Q_SLOTS:
        void toggleSearchbar();
        void handleEditAction();
        void handleDeleteAction();
        void handleCurrentSelectionChanged(QModelIndex prevIndex);
        void handleDoubleClicked(QModelIndex index);


    public:
        GatelibraryContentWidget(GatelibraryTableModel* model, QWidget* parent = nullptr);

        void handleContextMenuRequested(const QPoint& pos);

        void activate(bool readOnly = false);

        void toggleReadOnlyMode(bool readOnly);

        void toggleSelection(bool selected);


        QString disabledIconStyle() const;

        QString enabledIconStyle() const;

        QString addTypeIconPath() const;

        QString editTypeIconPath() const;

        QString searchIconPath() const;

        QString searchIconStyle() const;

        QString searchActiveIconStyle() const;

        QString deleteIconPath() const;

        QString deleteIconStyle() const;

        void setDisabledIconStyle(const QString& s);

        void setEnabledIconStyle(const QString& s);

        void setAddTypeIconPath(const QString& s);

        void setEditTypeIconPath(const QString& s);

        void setSearchIconPath(const QString& s);

        void setSearchIconStyle(const QString& s);

        void setSearchActiveIconStyle(const QString& s);

        void setDeleteIconPath(const QString& s);

        void setDeleteIconStyle(const QString& s);
    };

}
