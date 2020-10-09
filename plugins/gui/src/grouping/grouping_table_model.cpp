#include "gui/grouping/grouping_table_model.h"
#include "gui/gui_globals.h"

namespace hal {
    GroupingTableEntry::GroupingTableEntry(const QString& n, const QColor &c)
        : mGrouping(nullptr), mColor(c)
    {
        mGrouping = g_netlist->create_grouping(n.toStdString());
    }

    GroupingTableEntry::GroupingTableEntry(u32 existingId, const QColor& c)
        : mGrouping(nullptr), mColor(c)
    {
        mGrouping = g_netlist->get_grouping_by_id(existingId);
    }

    u32 GroupingTableEntry::id() const
    {
        if (!mGrouping) return 0;
        return mGrouping->get_id();
    }

    QString GroupingTableEntry::name() const
    {
        if (!mGrouping) return QString();
        return QString::fromStdString(mGrouping->get_name());
    }

    void GroupingTableEntry::setName(const QString &n)
    {
        if (!mGrouping) return;
        mGrouping->set_name(n.toStdString());
    }

    int GroupingTableModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent)
        return 3;
    }

    int GroupingTableModel::rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent)
        return mGroupings.size();
    }

    QVariant GroupingTableModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if(role != Qt::DisplayRole)
            return QVariant();

        if(orientation == Qt::Horizontal)
        {
            switch(section)
            {
                case 0: return "Grouping Name";
                case 1: return "Id";
                case 2: return "Color";
                default: break;
            }
            return QVariant();
        }

        return section + 1;
    }

    QVariant GroupingTableModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid()) return QVariant();
        const GroupingTableEntry& gte = mGroupings.at(index.row());

        switch (role) {
        case Qt::BackgroundRole:
            if (index.column()==2) return gte.color();
            return QVariant();
        case Qt::DisplayRole:
            switch (index.column()) {
            case 0:
                return gte.name();
            case 1:
                return gte.id();
            }
        default:
            break;
        }
        return QVariant();
    }

    bool GroupingTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if (role != Qt::EditRole) return false;
        if (!index.isValid()) return false;
        switch (index.column()) {
        case 0:
            mGroupings[index.row()].setName(value.toString().trimmed());
            return true;
        case 2:
            mGroupings[index.row()].setColor(value.value<QColor>());
            return true;
        }
        return false;
    }

    bool GroupingTableModel::removeRows(int row, int count, const QModelIndex &parent)
    {
        Q_UNUSED(parent);
        Q_UNUSED(count);

        if (row >= mGroupings.size()) return false;
        Q_EMIT layoutAboutToBeChanged();
        mGroupings.removeAt(row);
        Q_EMIT layoutChanged();
        return true;
    }

    bool GroupingTableModel::validate(const QString &input)
    {
        if (input.isEmpty()) return false;
        if (input == mAboutToRename) return true; // allow existing name on rename
        for (const GroupingTableEntry& gte : mGroupings)
        {
            if (gte.name() == input.trimmed())
                return false;
        }
        return true;
    }

    Grouping *GroupingTableModel::addDefaultEntry()
    {
        QSet<QString> existingNames;
        u32 maxId = 0;
        for (const GroupingTableEntry& gte : mGroupings)
        {
            if (gte.id()>maxId) maxId = gte.id();
            existingNames.insert(gte.name());
        }
        Q_EMIT layoutAboutToBeChanged();

        int baseCol = mGroupings.size() * 37;
        GroupingTableEntry gte(generateUniqueName(QString("grouping%1").arg(maxId+1),existingNames),
                               QColor::fromHsv(baseCol%255,200,std::max(250-baseCol/255*50,50)));
        mGroupings.append(gte);
        Q_EMIT layoutChanged();
        return gte.grouping();
    }

    QString GroupingTableModel::generateUniqueName(const QString& suggestion, const QSet<QString>& existingNames)
    {
        QString retval = suggestion;
        int itry = 0;
        while (existingNames.contains(retval) && itry < 27*26)
        {
            retval = suggestion;
            if (itry >= 26) retval += (char) ('a' + itry/26 -1);
            retval += (char) ('a' + itry%26);
            ++itry;
        }
        return retval;
    }

    QColor GroupingTableModel::colorForItem(item_type itemType, u32 itemId) const
    {
        Grouping* itemGrouping = nullptr;
        const Module* m = nullptr;
        const Gate* g = nullptr;
        const Net* n = nullptr;
        switch (itemType) {
        case item_type::module:
            m = g_netlist->get_module_by_id(itemId);
            if (m) itemGrouping = m->get_grouping();
            break;
        case item_type::gate:
            g = g_netlist->get_gate_by_id(itemId);
            if (g) itemGrouping = g->get_grouping();
            break;
        case item_type::net:
            n = g_netlist->get_net_by_id(itemId);
            if (n) itemGrouping = n->get_grouping();
            break;
        }
        if (itemGrouping)
        {
            for (const GroupingTableEntry& gte : mGroupings)
                if (gte.grouping() == itemGrouping)
                    return gte.color();

        }
        return QColor();
    }

    Grouping* GroupingTableModel::groupingByName(const QString& name) const
    {
        for (const GroupingTableEntry& gte : mGroupings)
            if (gte.name() == name)
                return gte.grouping();
        return nullptr;
    }

    QStringList GroupingTableModel::groupingNames() const
    {
        QStringList retval;
        for (const GroupingTableEntry& gte : mGroupings)
            retval << gte.name();
        return retval;
    }

}
