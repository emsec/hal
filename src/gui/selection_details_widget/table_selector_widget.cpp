#include "selection_details_widget/table_selector_widget.h"
#include "selection_details_widget/gate_details_widget.h"
#include <QDebug>
#include <QHeaderView>
#include <QKeyEvent>
#include <QScrollBar>
#include <gui_globals.h>

table_selector_widget::table_selector_widget(QWidget* parent) : QTableWidget(parent)
{
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
}

table_selector_widget::table_selector_widget(std::vector<endpoint> table_data, QWidget* parent) : QTableWidget(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);

    setShowGrid(false);
    verticalHeader()->setVisible(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setColumnCount(4);
    setRowCount(table_data.size());
    setHorizontalHeaderLabels(QStringList() << "ID"
                                            << "Name"
                                            << "Type"
                                            << "Pin");
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    //setStyleSheet("QTableView {background-color: black; color: white; selection-background-color: grey;} QTableView::item {border-right : 1px solid rgb(169, 183, 198);};");

    int counter = 0;
    for (const auto& ep : table_data)
    {
        setItem(counter, 0, new QTableWidgetItem(QString::number(ep.gate->get_id())));
        setItem(counter, 1, new QTableWidgetItem(QString::fromStdString(ep.gate->get_name())));
        setItem(counter, 2, new QTableWidgetItem(QString::fromStdString(ep.gate->get_type())));
        setItem(counter, 3, new QTableWidgetItem(QString::fromStdString(ep.pin_type)));
        counter++;
    }

    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    resizeColumnsToContents();
    resizeRowsToContents();

    QSize actual_size = calculate_actual_table_size();
    setMinimumWidth(actual_size.width());
    setMaximumWidth(actual_size.width());
    setMinimumHeight(((actual_size.height() > 400) ? 400 : actual_size.height()));
    setMaximumHeight(((actual_size.height() > 400) ? 400 : actual_size.height()));

    selectRow(0);

    connect(this, &QTableWidget::itemDoubleClicked, this, &table_selector_widget::on_item_double_clicked);
}

void table_selector_widget::focusOutEvent(QFocusEvent* event)
{
    Q_UNUSED(event)
    this->close();
}

void table_selector_widget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
        close();

    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return || event->key() == Qt::Key_Right)
    {
        auto gate                 = g_netlist->get_gate_by_id(this->item(selectedItems().first()->row(), 0)->text().toInt());
        auto pin                  = this->item(selectedItems().first()->row(), 3)->text().toStdString();
        Q_EMIT gateSelected({gate, pin});
    }

    QTableWidget::keyPressEvent(event);
}

QSize table_selector_widget::calculate_actual_table_size()
{
    int width = 4;
    for (int i = 0; i < columnCount(); i++)
        width += columnWidth(i);

    int height = horizontalHeader()->height() + 8;
    for (int i = 0; i < rowCount(); i++)
        height += rowHeight(i);

    return QSize(width, height - 6);
}

void table_selector_widget::on_item_double_clicked(QTableWidgetItem* item)
{
    auto gate = g_netlist->get_gate_by_id(this->item(item->row(), 0)->text().toInt());
    auto pin  = this->item(item->row(), 3)->text().toStdString();
    Q_EMIT gateSelected({gate, pin});

    //the signal does not reach the gate_details_widget even though it is connected in the gate_detail_widget class...
    //--->need to make the signal/slot mechanic work, this is just ugly (need to rely on the parent), also only the gate_details_widget is allowed to be a parent
    //static_cast<gate_details_widget*>(parent())->on_gate_selected(gate);
    //close();
}
