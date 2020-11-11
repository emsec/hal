#include <QHeaderView>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include "gui/selection_details_widget/data_fields_table.h"
#include "hal_core/netlist/data_container.h"

namespace hal {

    DataFieldsTable::DataFieldsTable(DetailsWidget *parent)
        : QTableWidget(0,2,parent), m_itemId(0)
    {
        mKeyFont = QFont("Iosevka");
        mKeyFont.setBold(true);
        mKeyFont.setPixelSize(13);
        m_pythonType = parent->detailsTypeName();
        connect(this, &QTableWidget::customContextMenuRequested, this, &DataFieldsTable::handleContextMenuRequest);
    }

    void DataFieldsTable::updateData(const u32 id, const std::map<std::tuple<std::string, std::string>,
                                                                  std::tuple<std::string, std::string>>& dataMap)
    {
        m_itemId = id;
        clearContents();
        setRowCount(dataMap.size());
        setMaximumHeight(verticalHeader()->length());
        int irow = 0;
        for(const auto& [key, value] : dataMap)
        {
            QString keyCategory = QString::fromStdString(std::get<0>(key));
            QString keyTxt      = QString::fromStdString(std::get<1>(key)) + ":";
            QString valueTxt    = QString::fromStdString(std::get<1>(value));
            QTableWidgetItem* key_item = new QTableWidgetItem(keyTxt);
            key_item->setFont(mKeyFont);
            key_item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            key_item->setData(Qt::UserRole, keyCategory);
            QTableWidgetItem* value_item = new QTableWidgetItem(valueTxt);
            value_item->setFlags(Qt::ItemIsEnabled);

            setItem(irow, 0, key_item);
            setItem(irow, 1, value_item);
            ++irow;
        }
        resizeColumnsToContents();
 //       setFixedWidth(calculate_table_size(m_data_fields_table).width());

    }

    void DataFieldsTable::handleContextMenuRequest(const QPoint &pos)
    {
        if(!itemAt(pos) || itemAt(pos)->column() != 1)
            return;

        QMenu menu;
        menu.addAction(QIcon(":/icons/python"), "Exctract data as python code (copy to clipboard)", [this, pos](){
            int row = itemAt(pos)->row();
            QString txt = item(row, 0)->text();
            QString key = txt.left(txt.length()-1);
            QApplication::clipboard()->setText( QString("netlist.get_%1_by_id").arg(m_pythonType)
                                                + "(" + QString::number(m_itemId)
                                                + ").data[(\"" + item(row, 0)->data(Qt::UserRole).toString()
                                                + "\", \"" + key + "\")]");//(’generic’, ’data’)
        });

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();

    }

}
