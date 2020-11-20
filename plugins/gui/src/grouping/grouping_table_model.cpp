#include "gui/grouping/grouping_table_model.h"
#include "gui/gui_globals.h"
#include <QApplication>

namespace hal {
    GroupingTableEntry::GroupingTableEntry(const QString& n, const QColor &c)
        : mGrouping(nullptr), mColor(c)
    {
        mGrouping = gNetlist->create_grouping(n.toStdString());
    }

    GroupingTableEntry::GroupingTableEntry(u32 existingId, const QColor& c)
        : mGrouping(nullptr), mColor(c)
    {
        mGrouping = gNetlist->get_grouping_by_id(existingId);
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


    GroupingTableModel::GroupingTableModel(QObject* parent)
        : QAbstractTableModel(parent), mDisableEvents(false)
    {
        //on creation load all already existing groupings from the netlist into the model
        for(auto grp : gNetlist->get_groupings())
        {
            mDisableEvents = true;
            Q_EMIT layoutAboutToBeChanged();
            GroupingTableEntry gte(grp->get_id(), nextColor());
            int n = mGroupings.size();
            mGroupings.append(gte);
            Q_EMIT layoutChanged();
            mDisableEvents = false;

            QModelIndex inx = index(n,0);
            Q_EMIT newEntryAdded(inx);
        }

        connect(gNetlistRelay, &NetlistRelay::groupingCreated, this, &GroupingTableModel::createGroupingEvent);
        connect(gNetlistRelay, &NetlistRelay::groupingRemoved, this, &GroupingTableModel::deleteGroupingEvent);
        connect(gNetlistRelay, &NetlistRelay::groupingNameChanged, this, &GroupingTableModel::groupingNameChangedEvent);
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
                case 1: return "ID";
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
            Q_EMIT groupingColorChanged(mGroupings.at(index.row()).grouping());
            return true;
        }
        return false;
    }

    bool GroupingTableModel::removeRows(int row, int count, const QModelIndex &parent)
    {
        Q_UNUSED(parent);
        Q_UNUSED(count);

        mDisableEvents = true;
        int nrows = mGroupings.size();
        if (row >= nrows) return false;
        Grouping* grp = mGroupings.at(row).grouping();
        for (Module* m : grp->get_modules())
            grp->remove_module(m);
        for (Gate* g : grp->get_gates())
            grp->remove_gate(g);
        for (Net* n : grp->get_nets())
            grp->remove_net(n);
        gNetlist->delete_grouping(grp);
        Q_EMIT layoutAboutToBeChanged();
        mGroupings.removeAt(row);
        Q_EMIT layoutChanged();
        mDisableEvents = false;
        if (row >= nrows-1)
            Q_EMIT lastEntryDeleted();
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

    Grouping* GroupingTableModel::addDefaultEntry()
    {
        QSet<QString> existingNames;
        u32 maxId = 0;
        for (const GroupingTableEntry& gte : mGroupings)
        {
            if (gte.id()>maxId) maxId = gte.id();
            existingNames.insert(gte.name());
        }

        mDisableEvents = true;
        Q_EMIT layoutAboutToBeChanged();
        GroupingTableEntry gte(generateUniqueName(QString("grouping%1").arg(maxId+1),existingNames),
                               nextColor());
        int n = mGroupings.size();
        mGroupings.append(gte);
        Q_EMIT layoutChanged();
        mDisableEvents = false;

        QModelIndex inx = index(n,0);
        Q_EMIT newEntryAdded(inx);
        return gte.grouping();
    }

    QColor GroupingTableModel::nextColor() const
    {
        int baseCol = mGroupings.size() * 37;
        return QColor::fromHsv(baseCol%255,200,std::max(250-baseCol/255*50,50));
    }

    void GroupingTableModel::deleteGroupingEvent(Grouping* grp)
    {
        if (mDisableEvents) return;
        for (auto it = mGroupings.begin(); it!=mGroupings.end(); ++it)
            if (it->grouping() == grp)
            {
                Q_EMIT layoutAboutToBeChanged();
                it = mGroupings.erase(it);
                Q_EMIT layoutChanged();
                if (it == mGroupings.end()) Q_EMIT lastEntryDeleted();
                return;
            }
    }

    void GroupingTableModel::createGroupingEvent(Grouping *grp)
    {
        if (mDisableEvents) return;
        Q_EMIT layoutAboutToBeChanged();
        int n = mGroupings.size();
        mGroupings.append(GroupingTableEntry(grp,nextColor()));
        Q_EMIT layoutChanged();
        QModelIndex inx = index(n,0);
        Q_EMIT newEntryAdded(inx);
    }

    void GroupingTableModel::groupingNameChangedEvent(Grouping *grp)
    {
        if (mDisableEvents) return;
        int irow = 0;
        for (auto it = mGroupings.begin(); it!=mGroupings.end(); ++it)
        {
            if (it->grouping() == grp)
            {
                it->setName(QString::fromStdString(grp->get_name()));
                QModelIndex inx = index(irow,0);
                Q_EMIT dataChanged(inx,inx);
            }
            ++irow;
        }
    }

    void GroupingTableModel::renameGrouping(int irow, const QString& groupingName)
    {
        mDisableEvents = true;
        Grouping* grp = mGroupings.at(irow).grouping();
        grp->set_name(groupingName.toStdString());
        mGroupings[irow].setName(groupingName);
        QModelIndex inx = index(irow,0);
        Q_EMIT dataChanged(inx,inx);
        mDisableEvents = false;
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

    QColor GroupingTableModel::colorForItem(ItemType itemType, u32 itemId) const
    {
        Grouping* itemGrouping = nullptr;
        const Module* m = nullptr;
        const Gate* g = nullptr;
        const Net* n = nullptr;
        switch (itemType) {
        case ItemType::Module:
            m = gNetlist->get_module_by_id(itemId);
            if (m) itemGrouping = m->get_grouping();
            break;
        case ItemType::Gate:
            g = gNetlist->get_gate_by_id(itemId);
            if (g) itemGrouping = g->get_grouping();
            break;
        case ItemType::Net:
            n = gNetlist->get_net_by_id(itemId);
            if (n) itemGrouping = n->get_grouping();
            break;
        default:
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
