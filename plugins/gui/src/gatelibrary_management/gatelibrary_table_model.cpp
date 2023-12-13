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
                    case 0: return QString::fromStdString(mEntries[index.row()]->get_name());
                    case 1: return mEntries[index.row()]->get_id();

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
        mEntries.clear();

        for (auto elem : g->get_gate_types())
        {
            mEntries.append(elem.second);
        }

        std::sort(mEntries.begin(), mEntries.end(), [](GateType* a, GateType* b)
        {
            return a->get_name() < b->get_name();
        });

        endResetModel();
    }

    GateType* GatelibraryTableModel::getGateTypeAtIndex(int index)
    {
        if(index >= mEntries.size())
            return nullptr;

        return mEntries[index];
    }
}
