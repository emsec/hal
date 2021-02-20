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

    /**
     * The table model used for groupings within the GroupingsManagerWidget. <br>
     * Read the Qt documentation for further descriptions of the table model system.
     */
    class GroupingTableModel : public QAbstractTableModel, public Validator
    {
        Q_OBJECT

        bool mDisableEvents;

        QList<GroupingTableEntry> mGroupings;
        QString mAboutToRename;

        static QString generateUniqueName(const QString& suggestion, const QSet<QString>& existingNames);
    public:
        /**
         * Constructor.
         *
         * @param parent - The parent widget
         */
        GroupingTableModel(QObject* parent=nullptr);

        /**
         * Returns the amount of columns. Is always 3 (Grouping, ID, Color)
         *
         * @param parent - Then parent model index
         * @returns 3
         */
        int columnCount(const QModelIndex &parent=QModelIndex()) const override;

        /**
         * Returns the amount of rows in the table. Is equal to the amount of stored groupings.
         *
         * @param parent - The parent model index
         * @returns the amount of rows in the table.
         */
        int rowCount(const QModelIndex &parent=QModelIndex()) const override;

        /**
         * Returns the data stored under the given role for a given index in the table model.
         *
         * @param index - The index in the table
         * @param role - The access role
         * @returns the data
         */
        QVariant data(const QModelIndex &index, int role) const override;

        /**
         * Returns the header data fields, i.e. 'Groupings', 'ID' or 'Color'
         *
         * @param section - The section (column) index
         * @param orientation - The orientation of the table
         * @param role - The access role
         * @returns the header data field at the given position
         */
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

        /**
         * Sets the role data for the item at index to value.
         *
         * @param index - The model index
         * @param value - The value to set
         * @param role - The access role
         * @returns <b>true</b> on success
         */
        bool setData(const QModelIndex &index, const QVariant &value, int role) override;

        /**
         * Remove the row at index row.
         *
         * @param row - The row index
         * @param count - Unused and ignored (always one row is deleted at a time)
         * @param parent - Unused and ingnored
         * @returns <b>true</b> on success
         */
        bool removeRows(int row, int count=1, const QModelIndex &parent=QModelIndex()) override;

        /**
         * Returns <b>true</b> if <i>input</i> is a unique and valid grouping name. Returns <b>false</b> otherwise.
         *
         * @param input - A grouping name
         * @returns <b>true</b> if <i>input</i> is a unique and valid grouping name.
         *          Returns <b>false</b> otherwise
         */
        bool validate(const QString &input);

        /**
         * This function is used to prevent the 'validate'-function to return <b>false</b> while renaming a grouping to
         * its old name. Therefore the old name of the grouping will always considered as valid. <br>
         * To achieve this one has to call this function to store the old name.
         *
         * @param oldName - The old name of the grouping
         */
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
