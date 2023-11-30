#include "gui/gatelibrary_management/gatelibrary_table_model.h"

#include "gui/gui_globals.h"
#include "gui/plugin_relay/gui_plugin_manager.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/plugin_system/fac_extension_interface.h"

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

        return 2;
    }

    QVariant GatelibraryTableModel::data(const QModelIndex& index, int role) const
    {
        if(index.row() < mEntries.size() && index.column() < columnCount())
        {
            if (role == Qt::DisplayRole)
            {
                switch (index.column())
                {
                case 0: return mEntries[index.row()].name;
                case 1: return mEntries[index.row()].id;

                }
            }
            else if (role == Qt::TextAlignmentRole)
            {
                if (index.column() == 0 || index.column() == 1)
                {
                    Qt::Alignment a;
                    a.setFlag(Qt::AlignLeft);
                    a.setFlag(Qt::AlignVCenter);
                    return QVariant(a);
                }
                else
                    return Qt::AlignCenter;
            }
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
            case 1: return "ID";
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


    void GatelibraryTableModel::loadFile(const GateLibrary* g)
    {

        if (!g)
            return;
        beginResetModel();
        mEntries = {};
        for (auto elem : g->get_gate_types())
        {
            Entry newEntry;

            newEntry.name = QString::fromStdString(elem.second->get_name());
            newEntry.id = elem.second->get_id();

            mEntries.append(newEntry);
        }
        std::sort(mEntries.begin(), mEntries.end(), [](Entry a, Entry b)
        {
            return a.name < b.name;
        });
        endResetModel();
    }
}
