#include "gui/gatelibrary_management/gatelibrary_table_model.h"

#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"

#include "gui/gui_globals.h"

#include <QDateTime>

namespace hal
{

    GatelibraryTableModel::GatelibraryTableModel(QObject* parent) : QAbstractTableModel(parent)
    {
    }

    int GatelibraryTableModel::rowCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)

        return mEntries.size();
    }

    int GatelibraryTableModel::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)

        return 4;
    }

    QVariant GatelibraryTableModel::data(const QModelIndex& index, int role) const
    {
        if(index.row() < mEntries.size() && index.column() < 4)
        {
            if (role == Qt::DisplayRole)
            {
                switch (index.column())
                {
                case 0: return mEntries[index.row()].name;
                case 1: return mEntries[index.row()].gatecount;
                case 2: return QString::fromStdString(mEntries[index.row()].path.string());
                case 3: return mEntries[index.row()].timestring;
                }
            }
            else if (role == Qt::TextAlignmentRole)
            {
                if (index.column() == 0 || index.column() == 2)
                {
                    Qt::Alignment a;
                    a.setFlag(Qt::AlignLeft);
                    a.setFlag(Qt::AlignVCenter);
                    return QVariant(a);
                }
                else
                    return Qt::AlignCenter;
            }
            else if (role == Qt::UserRole)
                if (index.column() == 3)
                    return mEntries[index.row()].timevalue;
        }

        return QVariant();
    }

    QVariant GatelibraryTableModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if(role != Qt::DisplayRole)
            return QVariant();

        if(orientation == Qt::Horizontal)
        {
            switch(section)
            {
            case 0: return "Name";
            case 1: return "Gate Count";
            case 2: return "Path";
            case 3: return "Load Time";
            default: return QVariant();
            }
        }

        return section + 1;
    }

    void GatelibraryTableModel::clear()
    {
        beginResetModel();
        mEntries.clear();
        endResetModel();
    }

    void GatelibraryTableModel::setup()
    {
        QList<Entry> newEntryList;

        for (GateLibrary* g : gate_library_manager::get_gate_libraries())
        {
            Entry newEntry;

            newEntry.name = QString::fromStdString(g->get_name());
            newEntry.gatecount = (u32)g->get_gate_types().size();
            newEntry.path = g->get_path();
            newEntry.timestring = QString::fromStdString("Program Start");
            newEntry.timevalue = 0;

            newEntryList.append(newEntry);
        }

        beginResetModel();
        mEntries = newEntryList;
        endResetModel();
    }

    QString GatelibraryTableModel::getNameFromIndex(const QModelIndex& index)
    {
        return mEntries[index.row()].name;
    }

    u32 GatelibraryTableModel::getGatecountFromIndex(const QModelIndex& index)
    {
        return mEntries[index.row()].gatecount;
    }

    void GatelibraryTableModel::loadFile(const QString& path)
    {
        std::filesystem::path stdPath = path.toStdString();
        GateLibrary* g = gate_library_manager::load(stdPath, false);

        if (!g)
            return;

        beginInsertRows(QModelIndex(), rowCount(), rowCount());

        Entry newEntry;

        newEntry.name = QString::fromStdString(g->get_name());
        newEntry.gatecount = (u32)g->get_gate_types().size();
        newEntry.path = g->get_path();
        newEntry.timestring = QDateTime::currentDateTime().toString();
        newEntry.timevalue = QDateTime::currentDateTime().toMSecsSinceEpoch();

        mEntries.append(newEntry);

        endInsertRows();
    }

    void GatelibraryTableModel::reloadIndex(const QModelIndex& index)
    {
        int row = index.row();

        assert(row < mEntries.size());

        gate_library_manager::load(mEntries[row].path, true);

        mEntries[row].timestring = QDateTime::currentDateTime().toString();
        mEntries[row].timevalue = QDateTime::currentDateTime().toMSecsSinceEpoch();

        QModelIndex changed = createIndex(row, 3, &mEntries[row]);
        Q_EMIT dataChanged(changed, changed);
    }

    void GatelibraryTableModel::removeIndex(const QModelIndex& index)
    {
        int row = index.row();

        assert(row < mEntries.size());

        gate_library_manager::remove(mEntries[row].path);

        beginRemoveRows(index.parent(), row, row);
        mEntries.removeAt(row);
        endRemoveRows();
    }
}
