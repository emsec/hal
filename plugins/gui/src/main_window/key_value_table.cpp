#include "gui/main_window/key_value_table.h"
#include "hal_core/utilities/json_write_document.h"
#include <QGridLayout>

namespace hal {
    KeyValueTable::KeyValueTable(const QString &labl, QWidget* parent)
        : QWidget(parent)
    {
        QGridLayout* layout = new QGridLayout(this);
        layout->addWidget(new QLabel(labl,this),0,0);
        mTable = new QTableWidget(this);
        mTable->setColumnCount(2);
        mTable->setRowCount(3);
        const char* headr[] = { "Key", "Value"};
        for (int icol=0; icol<2; icol++)
        {
            QTableWidgetItem* item = new QTableWidgetItem(headr[icol]);
            QFont fnt = item->font();
            fnt.setBold(true);
            fnt.setPointSize(14);
            item->setFont(fnt);
            mTable->setHorizontalHeaderItem(icol,item);
        }
        mTable->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        layout->addWidget(mTable,1,0);
    }

    void KeyValueTable::setJson(const std::string& jsonTxt)
    {
        std::unordered_map<std::string,std::string> dict = JsonConverter::stringToDictionary(jsonTxt);
        int nrows = dict.size() + 1;
        if (mTable->rowCount() < nrows) mTable->setRowCount(nrows);
        int irow = 0;
        for (auto it = dict.begin(); it != dict.end(); ++it)
        {
            mTable->setItem(irow,0,new QTableWidgetItem(QString::fromStdString(it->first)));
            mTable->setItem(irow,1,new QTableWidgetItem(QString::fromStdString(it->second)));
            ++irow;
        }
    }

    std::string KeyValueTable::toJson() const
    {
        std::unordered_map<std::string,std::string> dict;
        int nrows = mTable->rowCount();
        for (int irow=0; irow < nrows; irow++)
        {
            const QTableWidgetItem* item = mTable->item(irow,0);
            if (!item) continue;
            QString key = item->text();
            if (key.isEmpty()) continue;
            item = mTable->item(irow,1);
            dict[key.toStdString()] = item ? item->text().toStdString() : std::string();
        }
        return JsonConverter::dictionaryToString(dict);
    }
}
