#pragma once

#include <QAbstractTableModel>
#include <QString>
#include <QColor>
#include <QList>

#include "hal_core/defines.h"
#include "hal_core/netlist/grouping.h"
#include "gui/validator/validator.h"
#include "gui/graph_widget/items/graphics_item.h"

namespace hal {
    class GroupingTableEntry
    {
        Grouping* mGrouping;
        QColor  mColor;
    public:
        GroupingTableEntry(const QString& n, const QColor& c); // constructor for new grouping
        GroupingTableEntry(u32 existingId, const QColor& c);  // entry wraps existing group object
        GroupingTableEntry(Grouping* grp, const QColor&c)
            : mGrouping(grp), mColor(c) {;}
        u32 id() const;
        QString name() const;
        QColor color() const { return mColor; }
        void setName(const QString& n);
        void setColor(const QColor& c) { mColor = c; }
        Grouping* grouping() const { return mGrouping; }
    };


    class GroupingTableModel : public QAbstractTableModel, public Validator
    {
        Q_OBJECT

        bool mDisableEvents;
        QList<GroupingTableEntry> mGroupings;
        QString mAboutToRename;

        static QString generateUniqueName(const QString& suggestion, const QSet<QString>& existingNames);
    public:
        GroupingTableModel(QObject* parent=nullptr);
        int columnCount(const QModelIndex &parent=QModelIndex()) const override;
        int rowCount(const QModelIndex &parent=QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
        bool setData(const QModelIndex &index, const QVariant &value, int role) override;
        bool removeRows(int row, int count=1, const QModelIndex &parent=QModelIndex()) override;
        bool validate(const QString &input);
        void setAboutToRename(const QString& oldName) { mAboutToRename = oldName.trimmed(); }
        GroupingTableEntry groupingAt(int irow) const { return mGroupings.at(irow); }
        Grouping* addDefaultEntry();
        Grouping* groupingByName(const QString& name) const;
        QColor colorForItem(ItemType itemType, u32 itemId) const;
        void renameGrouping(int irow, const QString& groupingName);
        QStringList groupingNames() const;

    public Q_SLOTS:
        void deleteGroupingEvent(Grouping* grp);
        void createGroupingEvent(Grouping *grp);
        void groupingNameChangedEvent(Grouping *grp);

    Q_SIGNALS:
        void groupingColorChanged(Grouping* grp);
        void lastEntryDeleted();
        void newEntryAdded(QModelIndex& index);

    private:
        QColor nextColor() const;
    };
}
