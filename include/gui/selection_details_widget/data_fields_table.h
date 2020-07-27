#pragma once

#include "def.h"
#include <QTableWidget>
#include <QFont>
#include "selection_details_widget/details_widget.h"

namespace hal {

    class DataContainer;

    class DataFieldsTable : public QTableWidget
    {
        Q_OBJECT
    public:

        DataFieldsTable(DetailsWidget* parent = nullptr);
        void updateData(const u32 id, const std::map<std::tuple<std::string, std::string>,
                                                     std::tuple<std::string, std::string>>& dc);

    private Q_SLOTS:
        void handleContextMenuRequest(const QPoint &pos);

    private:
        u32     m_itemId;
        QString m_pythonType;
        QFont   m_keyFont;
    };
}

