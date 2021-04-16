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

    class ModuleSelectEntry
    {
        u32 mId;
        QString mName;
        QString mType;
        QColor mColor;
    public:
        ModuleSelectEntry(Module* m);
        QVariant data(int icol) const;
        QColor color() const { return mColor; }
        u32 id() const { return mId; }
    };

    class ModuleSelectExclude {
        QList<u32> mGates;
        QList<u32> mModules;
        QSet<u32> mExclude;
    public:
        ModuleSelectExclude();
        bool isAccepted(u32 modId) const { return !mExclude.contains(modId); }
        QString selectionToString() const;
        QSet<u32> modules() const { return QSet<u32>::fromList(mModules); }
        QSet<u32> gates()   const { return QSet<u32>::fromList(mGates);   }
    };

    class ModuleSelectModel : public QAbstractTableModel
    {
        Q_OBJECT

        QList<ModuleSelectEntry> mEntries;

    public:
        ModuleSelectModel(bool history, QObject* parent=nullptr);

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        u32 moduleId(int irow) const;
        QColor moduleColor(int irow) const;
    };

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

    class ModuleSelectPicker : public QObject
    {
        Q_OBJECT
        ModuleSelectExclude mSelectExclude;
    public:
        ModuleSelectPicker();

    Q_SIGNALS:
        void triggerModuleCursor(bool on);

    public Q_SLOTS:
        void handleSelectionChanged(void* sender);
    };

    class ModuleSelectHistory : public QList<u32>
    {
        static ModuleSelectHistory* inst;
        ModuleSelectHistory() {;}
    public:
        static ModuleSelectHistory* instance();
        void add(u32 id);
    };

    class ModuleSelectView : public QTableView
    {
        Q_OBJECT

    public:
        ModuleSelectView(bool history, Searchbar* sbar,
                          QWidget* parent=nullptr);
        void adjust();

    Q_SIGNALS:
        void moduleSelected(u32 modId, bool doubleClick);

    private Q_SLOTS:
        void handleCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
        void handleDoubleClick(const QModelIndex& index);
    };
}

