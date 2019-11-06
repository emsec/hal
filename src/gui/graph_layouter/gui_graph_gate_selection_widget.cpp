#include "graph_layouter/gui_graph_gate_selection_widget.h"
#include "graph_layouter/graph_layouter_view.h"

#include "core/log.h"

#include <QAbstractItemView>
#include <QDebug>
#include <QHeaderView>
#include <QKeyEvent>
#include <QString>
#include <QStringList>
#include <QTableWidgetItem>
#include "gui_globals.h"

gui_graph_gate_selection_widget::gui_graph_gate_selection_widget(QWidget* parent, std::vector<endpoint> successors, std::shared_ptr<gate> curr_gate, std::string curr_pin_type)
    : QTableWidget(parent), m_curr_gate(curr_gate), m_curr_pin(curr_pin_type)
{
    //the parent has to be a layouter_view, needs a rework, for now its sufficient
    m_view  = dynamic_cast<graph_layouter_view*>(parent);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);

    setShowGrid(false);
    verticalHeader()->setVisible(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setColumnCount(5);
    setRowCount(successors.size());
    setHorizontalHeaderLabels(QStringList() << "ID"
                                            << "Name"
                                            << "Gate Type"
                                            << "Pin Type"
                                            << "Module");
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto modules_of_gate = [](std::shared_ptr<gate> gate) -> QString {
        QString modules = "";
        bool first         = true;
        for (const auto& sub : gate->get_netlist()->get_modules())
        {
            if (sub->contains_gate(gate))
            {
                if (!first)
                    modules += ", ";
                first = false;
                modules += QString::fromStdString(sub->get_name()) + " (" + QString::number(sub->get_id()) + ")";
            }
        }
        return modules;
    };

    int counter = 0;
    for(const auto& tup : successors)
    {
        auto current_gate = tup.gate;
        setItem(counter, 0, new QTableWidgetItem(QString::number(current_gate->get_id())));
        setItem(counter, 1, new QTableWidgetItem(QString::fromStdString(current_gate->get_name())));
        setItem(counter, 2 , new QTableWidgetItem(QString::fromStdString(current_gate->get_type()->get_name())));
        setItem(counter, 3, new QTableWidgetItem(QString::fromStdString(tup.pin_type)));
        setItem(counter, 4, new QTableWidgetItem(modules_of_gate(current_gate)));
        counter++;
    }

    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    resizeColumnsToContents();
    resizeRowsToContents();

    QSize actual_size = table_widget_size();
    setMinimumWidth(actual_size.width());
    setMaximumWidth(actual_size.width());
    setMinimumHeight(((actual_size.height() > 400) ? 400 : actual_size.height()));
    setMaximumHeight(((actual_size.height() > 400) ? 400 : actual_size.height()));

    selectRow(0);

    connect(this, &QTableWidget::itemDoubleClicked, this, &gui_graph_gate_selection_widget::on_item_double_clicked);
}

//list is focused, so no need to implement down/up button, because the list automatically does the right thing for you
void gui_graph_gate_selection_widget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return || event->key() == Qt::Key_Right)
    {
        auto gate = g_netlist->get_gate_by_id(static_cast<const u32>(std::stoi(selectedItems().at(0)->text().toStdString())));

        //communicate with the view
        m_view->onSuccessorgateSelected(gate);
        Q_EMIT gateSelected(gate);
        this->close();
    }

    if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Left)
        this->close();

    QTableWidget::keyPressEvent(event);
}

void gui_graph_gate_selection_widget::set_is_net_selected(bool b)
{
    is_net_selected = b;
}

QSize gui_graph_gate_selection_widget::table_widget_size()
{
    int width = 4;
    for (int i = 0; i < columnCount(); i++)
        width += columnWidth(i);

    int height = horizontalHeader()->height() + 8;
    for (int i = 0; i < rowCount(); i++)
        height += rowHeight(i);

    return QSize(width, height - 6);
}

void gui_graph_gate_selection_widget::on_item_double_clicked(QTableWidgetItem *item)
{
    auto gate = g_netlist->get_gate_by_id(static_cast<const u32>(std::stoi(selectedItems().at(0)->text().toStdString())));
    m_view->onSuccessorgateSelected(gate);
    Q_EMIT gateSelected(gate);
    this->close();
}
