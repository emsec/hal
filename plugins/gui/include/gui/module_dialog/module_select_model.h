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
#include <QColor>
#include <QList>

namespace hal {

    class Module;
    class Searchbar;

    /**
     * @brief The ModuleSelectEntry class comprises a single entry of the module selection table
     */
    class ModuleSelectEntry
    {
        u32 mId;
        QString mName;
        QString mType;
        QColor mColor;
    public:
        ModuleSelectEntry(Module* m);

        /**
         * @brief data returns data from requested column
         * @param icol column number (0, 1, 2, 3)
         * @return 0=module color  1=Id  2=name   3=type
         */
        QVariant data(int icol) const;
        QColor color() const { return mColor; }
        u32 id() const { return mId; }
    };

    /**
     * @brief The ModuleSelectExclude class is used to determine which modules can't be selected
     *
     * Member variables are the gates and modules currently selected and a set of non-selectable
     * modules. The 'blacklisted' modules are identical with a selected one, or a (grand?-)child,
     * or a direct parent.
     */
    class ModuleSelectExclude {
        QList<u32> mGates;
        QList<u32> mModules;
        QSet<u32> mExclude;
    public:
        ModuleSelectExclude();
        bool isAccepted(u32 modId) const { return !mExclude.contains(modId); }

        /**
         * @brief selectionToString function is used to generate selection as text for message box
         * @return selected items as formatted string
         */
        QString selectionToString() const;
        QSet<u32> modules() const { return QSet<u32>::fromList(mModules); }
        QSet<u32> gates()   const { return QSet<u32>::fromList(mGates);   }
    };

    /**
     * @brief The ModuleSelectModel class is the source model for module selection
     */
    class ModuleSelectModel : public QAbstractTableModel
    {
        Q_OBJECT

        QList<ModuleSelectEntry> mEntries;

    public:
        /**
         * @brief ModuleSelectModel constructor
         * @param history if true a list of modules previously selected gets generated
         * @param parent the parent object
         */
        ModuleSelectModel(bool history, QObject* parent=nullptr);

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        u32 moduleId(int irow) const;
        QColor moduleColor(int irow) const;
    };

    /**
     * @brief The ModuleSelectProxy class allows sorting and filtering of module tables
     */
    class ModuleSelectProxy : public QSortFilterProxyModel
    {
        Q_OBJECT

    public:
        ModuleSelectProxy(QObject* parent = nullptr);

    public Q_SLOTS:
        void setSortMechanism(gui_utility::mSortMechanism sortMechanism);
        void searchTextChanged(const QString& txt);

    protected:
        static bool lessThan(const QColor& a, const QColor& b);
        bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override;

    private:
        gui_utility::mSortMechanism mSortMechanism;
    };

    /**
     * @brief The ModuleSelectPicker class instance gets spawned to pick module from graph
     */
    class ModuleSelectPicker : public QObject
    {
        Q_OBJECT
        ModuleSelectExclude mSelectExclude;
        static ModuleSelectPicker* sCurrentPicker;

    public:
        ModuleSelectPicker();

        static void terminateCurrentPicker();

    Q_SIGNALS:
        void triggerCursor(int icurs);

    public Q_SLOTS:
        void handleSelectionChanged(void* sender);
    };

    /**
     * @brief The ModuleSelectHistory class singleton comprises a list of user selected modules
     */
    class ModuleSelectHistory : public QList<u32>
    {
        static ModuleSelectHistory* inst;
        ModuleSelectHistory() {;}
    public:
        static ModuleSelectHistory* instance();
        void add(u32 id);
    };

    /**
     * @brief The ModuleSelectView class is the table widget for module selection
     */
    class ModuleSelectView : public QTableView
    {
        Q_OBJECT

    public:
        /**
         * @brief ModuleSelectView constructor
         * @param history if true a list of modules previously selected gets generated
         * @param sbar the filter-string editor to connect with
         * @param parent the parent widget
         */
        ModuleSelectView(bool history, Searchbar* sbar,
                          QWidget* parent=nullptr);

    Q_SIGNALS:
        void moduleSelected(u32 modId, bool doubleClick);

    private Q_SLOTS:
        void handleCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
        void handleDoubleClick(const QModelIndex& index);
    };
}

