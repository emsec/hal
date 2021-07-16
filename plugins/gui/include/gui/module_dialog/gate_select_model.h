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

#include "hal_core/defines.h"
#include "gui/gui_utils/sort.h"
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QSet>
#include <QList>

namespace hal {

    class Gate;
    class Module;
    class Searchbar;

    /**
     * @brief The GateSelectEntry class comprises a single entry of the module selection table
     */
    class GateSelectEntry
    {
        u32 mId;
        QString mName;
        QString mType;
    public:
        GateSelectEntry(Gate* g);

        /**
         * @brief data returns data from requested column
         * @param icol column number (0, 1, 2, 3)
         * @return 0=Id  1=name   2=type
         */
        QVariant data(int icol) const;
        u32 id() const { return mId; }
    };


    /**
     * @brief The GateSelectModel class is the source model for module selection
     */
    class GateSelectModel : public QAbstractTableModel
    {
        Q_OBJECT

        QList<GateSelectEntry> mEntries;
        QSet<u32> mSelectableGates;

    public:
        /**
         * @brief GateSelectModel constructor
         * @param history if true a list of modules previously selected gets generated
         * @param parent the parent object
         */
        GateSelectModel(bool history, const QSet<u32>& selectable = QSet<u32>(), QObject* parent=nullptr);

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        u32 gateId(int irow) const;
        static bool isAccepted(u32 gateId, const QSet<u32>& selectable);
    };

    /**
     * @brief The GateSelectProxy class allows sorting and filtering of module tables
     */
    class GateSelectProxy : public QSortFilterProxyModel
    {
        Q_OBJECT

    public:
        GateSelectProxy(QObject* parent = nullptr);

    public Q_SLOTS:
        void setSortMechanism(gui_utility::mSortMechanism sortMechanism);
        void searchTextChanged(const QString& txt);

    protected:
        bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override;

    private:
        gui_utility::mSortMechanism mSortMechanism;
    };

    /**
     * @brief The GateSelectPicker class instance gets spawned to pick module from graph
     */
    class GateSelectPicker : public QObject
    {
        Q_OBJECT
        u32 mOrigin;
        bool mPickSuccessor;
        QSet<u32> mSelectableGates;
        static GateSelectPicker* sCurrentPicker;

    public:
        GateSelectPicker(u32 orig, bool succ, const QSet<u32>& selectable);

        static void terminateCurrentPicker();

    Q_SIGNALS:
        void triggerCursor(int icurs);
        void gatesPicked(u32 idFrom, u32 idTo);

    public Q_SLOTS:
        void handleSelectionChanged(void* sender);
    };

    /**
     * @brief The GateSelectHistory class singleton comprises a list of user selected modules
     */
    class GateSelectHistory : public QList<u32>
    {
        static GateSelectHistory* inst;
        GateSelectHistory() {;}
    public:
        static GateSelectHistory* instance();
        void add(u32 id);
    };

    /**
     * @brief The GateSelectView class is the table widget for module selection
     */
    class GateSelectView : public QTableView
    {
        Q_OBJECT

    public:
        /**
         * @brief GateSelectView constructor
         * @param history if true a list of modules previously selected gets generated
         * @param sbar the filter-string editor to connect with
         * @param parent the parent widget
         */
        GateSelectView(bool history, Searchbar* sbar, const QSet<u32>& selectable,
                          QWidget* parent=nullptr);

    Q_SIGNALS:
        void gateSelected(u32 gatId, bool doubleClick);

    private Q_SLOTS:
        void handleCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
        void handleDoubleClick(const QModelIndex& index);
    };
}

