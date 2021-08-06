#pragma once

#include <QDialog>
#include <QAbstractTableModel>
#include <QTableView>
#include <QStyledItemDelegate>
#include "wave_data.h"

namespace hal {

    class WaveEditTable : public QAbstractTableModel
    {
        Q_OBJECT
        WaveData mWaveData;
    public:
        WaveData* dataFactory() const;
        WaveEditTable(const WaveData& wd, QObject* parent = nullptr);
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        void removeRow(int irow);
    };

    class WaveDeleteDelegate : public QStyledItemDelegate
    {
        Q_OBJECT
        WaveEditTable* mTable;
    public:
        WaveDeleteDelegate(WaveEditTable*tab) : mTable(tab) {;}

        void paint(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

        QSize sizeHint(const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
    private Q_SLOTS:
            void deleteClicked();
    };

    class WaveEditDialog : public QDialog
    {
        Q_OBJECT
        WaveEditTable* mWaveModel;
    public:
        WaveEditDialog(const WaveData& wd, QWidget* parent = nullptr);
        WaveData* dataFactory() const;
    };

}
