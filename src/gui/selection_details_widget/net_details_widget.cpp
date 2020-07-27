#include "selection_details_widget/net_details_widget.h"
#include "selection_details_widget/disputed_big_icon.h"

#include "gui_globals.h"
#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/net.h"
#include "input_dialog/input_dialog.h"

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QMenu>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace hal
{
    NetDetailsWidget::NetDetailsWidget(QWidget* parent) : QWidget(parent)
    {
        //general initializations
        init_settings();
        m_current_id = 0;
        m_key_font   = QFont("Iosevka");
        m_key_font.setBold(true);
        m_key_font.setPixelSize(13);

        m_scroll_area       = new QScrollArea();
        m_top_lvl_container = new QWidget();
        m_top_lvl_layout    = new QVBoxLayout(m_top_lvl_container);
        m_top_lvl_container->setLayout(m_top_lvl_layout);
        m_scroll_area->setWidget(m_top_lvl_container);
        m_scroll_area->setWidgetResizable(true);
        m_content_layout = new QVBoxLayout(this);

        //layout customization
        m_content_layout->setContentsMargins(0, 0, 0, 0);
        m_content_layout->setSpacing(0);
        m_top_lvl_layout->setContentsMargins(0, 0, 0, 0);
        m_top_lvl_layout->setSpacing(0);

        //intermediate layout for the 3 sections (to add left spacing)
        QHBoxLayout* intermediate_layout_gt = new QHBoxLayout();
        intermediate_layout_gt->setContentsMargins(3, 3, 0, 0);
        intermediate_layout_gt->setSpacing(0);
        QHBoxLayout* intermediate_layout_sources = new QHBoxLayout();
        intermediate_layout_sources->setContentsMargins(3, 3, 0, 0);
        intermediate_layout_sources->setSpacing(10);
        QHBoxLayout* intermediate_layout_destinations = new QHBoxLayout();
        intermediate_layout_destinations->setContentsMargins(3, 3, 0, 0);
        intermediate_layout_destinations->setSpacing(0);

        //buttons
        m_general_info_button = new QPushButton("Net Information", this);
        m_general_info_button->setEnabled(false);
        m_source_pins_button      = new QPushButton("Source Pins", this);
        m_destination_pins_button = new QPushButton("Destination Pins", this);

        //table initializations
        m_general_table          = new QTableWidget(3, 2);
        m_source_pins_table      = new QTableWidget(0, 3);
        m_destination_pins_table = new QTableWidget(0, 3);

        QList<QTableWidget*> tmp;
        tmp << m_general_table << m_source_pins_table << m_destination_pins_table;
        for (auto& table : tmp)
        {
            table->horizontalHeader()->hide();
            table->verticalHeader()->hide();
            table->verticalHeader()->setDefaultSectionSize(16);
            table->resizeColumnToContents(0);
            table->setShowGrid(false);
            table->setFocusPolicy(Qt::NoFocus);
            table->setFrameStyle(QFrame::NoFrame);
            table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            table->setMaximumHeight(table->verticalHeader()->length());
            table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            table->setContextMenuPolicy(Qt::CustomContextMenu);
        }

        QList<QTableWidgetItem*> tmp_general_info_list = {new QTableWidgetItem("Name:"), new QTableWidgetItem("Type:"), new QTableWidgetItem("ID:")};
        for (int i = 0; i < tmp_general_info_list.size(); i++)
        {
            auto item = tmp_general_info_list.at(i);
            item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            item->setFont(m_key_font);
            m_general_table->setItem(i, 0, item);
        }

        //create dynamic items that change when gate is changed
        m_name_item = new QTableWidgetItem();
        m_name_item->setFlags(Qt::ItemIsEnabled);
        m_general_table->setItem(0, 1, m_name_item);

        m_type_item = new QTableWidgetItem();
        m_type_item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
        m_general_table->setItem(1, 1, m_type_item);

        m_id_item = new QTableWidgetItem();
        m_id_item->setFlags(Qt::ItemIsEnabled);
        m_general_table->setItem(2, 1, m_id_item);

        // place net icon
        QLabel* img = new DisputedBigIcon("sel_net", this);

        //adding things to intermediate layout (the one thats neccessary for the left spacing)
        intermediate_layout_gt->addWidget(m_general_table);
        intermediate_layout_gt->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
        intermediate_layout_gt->addWidget(img);
        intermediate_layout_gt->setAlignment(img,Qt::AlignTop);
        intermediate_layout_sources->addWidget(m_source_pins_table);
        intermediate_layout_sources->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
        intermediate_layout_destinations->addWidget(m_destination_pins_table);
        intermediate_layout_destinations->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

        //adding things to the main layout
        m_top_lvl_layout->addWidget(m_general_info_button);
        m_top_lvl_layout->addLayout(intermediate_layout_gt);
        m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 7, QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_top_lvl_layout->addWidget(m_source_pins_button);
        m_top_lvl_layout->addLayout(intermediate_layout_sources);
        m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 7, QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_top_lvl_layout->addWidget(m_destination_pins_button);
        m_top_lvl_layout->addLayout(intermediate_layout_destinations);

        //necessary to add at the end
        m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
        m_content_layout->addWidget(m_scroll_area);

        //connect the buttons
        connect(m_source_pins_button, &QPushButton::clicked, this, &NetDetailsWidget::handle_buttons_clicked);
        connect(m_destination_pins_button, &QPushButton::clicked, this, &NetDetailsWidget::handle_buttons_clicked);

        //connect the tables
        connect(m_source_pins_table, &QTableWidget::itemDoubleClicked, this, &NetDetailsWidget::handle_table_item_clicked);
        connect(m_destination_pins_table, &QTableWidget::itemDoubleClicked, this, &NetDetailsWidget::handle_table_item_clicked);
        connect(m_general_table, &QTableWidget::customContextMenuRequested, this, &NetDetailsWidget::handle_general_table_menu_requeted);
        connect(m_source_pins_table, &QTableWidget::customContextMenuRequested, this, &NetDetailsWidget::handle_sources_table_menu_requeted);
        connect(m_destination_pins_table, &QTableWidget::customContextMenuRequested, this, &NetDetailsWidget::handle_destinations_table_menu_requeted);

        //settings
        connect(&g_settings_relay, &SettingsRelay::setting_changed, this, &NetDetailsWidget::handle_global_settings_changed);

        //install eventfilter to change the cursor when hovering over the second colums of the pin tables
        m_destination_pins_table->viewport()->setMouseTracking(true);
        m_destination_pins_table->viewport()->installEventFilter(this);
        m_source_pins_table->viewport()->setMouseTracking(true);
        m_source_pins_table->viewport()->installEventFilter(this);

        //NetlistRelay connections
        connect(&g_netlist_relay, &NetlistRelay::net_removed, this, &NetDetailsWidget::handle_net_removed);
        connect(&g_netlist_relay, &NetlistRelay::net_name_changed, this, &NetDetailsWidget::handle_net_name_changed);
        connect(&g_netlist_relay, &NetlistRelay::net_source_added, this, &NetDetailsWidget::handle_net_source_added);
        connect(&g_netlist_relay, &NetlistRelay::net_source_removed, this, &NetDetailsWidget::handle_net_source_removed);
        connect(&g_netlist_relay, &NetlistRelay::net_destination_added, this, &NetDetailsWidget::handle_net_destination_added);
        connect(&g_netlist_relay, &NetlistRelay::net_destination_removed, this, &NetDetailsWidget::handle_net_destination_removed);
        connect(&g_netlist_relay, &NetlistRelay::gate_name_changed, this, &NetDetailsWidget::handle_gate_name_changed);
    }

    NetDetailsWidget::~NetDetailsWidget()
    {
    }

    bool NetDetailsWidget::eventFilter(QObject* watched, QEvent* event)
    {
        //need to determine which of the tables is the "owner" of the viewport
        QTableWidget* table = (watched == m_destination_pins_table->viewport()) ? m_destination_pins_table : m_source_pins_table;
        if (event->type() == QEvent::MouseMove)
        {
            QMouseEvent* ev        = dynamic_cast<QMouseEvent*>(event);
            QTableWidgetItem* item = table->itemAt(ev->pos());
            if (item)
            {
                if (item->column() == 2)
                    setCursor(QCursor(Qt::PointingHandCursor));
                else
                    setCursor(QCursor(Qt::ArrowCursor));
            }
            else
                setCursor(QCursor(Qt::ArrowCursor));
        }

        //restore default cursor when leaving the widget (maybe save cursor before entering?)
        if (event->type() == QEvent::Leave)
            setCursor(QCursor(Qt::ArrowCursor));

        return false;
    }

    void NetDetailsWidget::update(u32 net_id)
    {
        m_current_id = net_id;
        auto n       = g_netlist->get_net_by_id(net_id);

        if (m_current_id == 0 || !n)
            return;

        show_all_sections();

        // (1) update general info section
        m_name_item->setText(QString::fromStdString(n->get_name()));
        m_id_item->setText(QString::number(m_current_id));

        //get net type
        QString n_type = "Standard";

        if (g_netlist->is_global_input_net(n))
            n_type = "Input";
        else if (g_netlist->is_global_output_net(n))
            n_type = "Output";

        m_type_item->setText(n_type);
        m_general_table->resizeColumnsToContents();
        m_general_table->setFixedWidth(calculate_table_size(m_general_table).width());

        // (2) update sources section
        m_source_pins_table->clearContents();
        m_source_pins_button->setText("Sources (" + QString::number(n->get_sources().size()) + ")");
        m_source_pins_table->setRowCount(n->get_sources().size());
        m_source_pins_table->setMaximumHeight(m_source_pins_table->verticalHeader()->length());
        int index = 0;
        if (!g_netlist->is_global_input_net(n))
        {
            for (const auto& ep_source : n->get_sources())
            {
                QTableWidgetItem* pin_name       = new QTableWidgetItem(QString::fromStdString(ep_source.get_pin()));
                QTableWidgetItem* arrow_item     = new QTableWidgetItem(QChar(0x2b05));
                QTableWidgetItem* gate_name_item = new QTableWidgetItem(QString::fromStdString(ep_source.get_gate()->get_name()));
                arrow_item->setForeground(QBrush(QColor(114, 140, 0), Qt::SolidPattern));
                pin_name->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
                arrow_item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
                //arrow_item->setFlags((Qt::ItemFlag)(~Qt::ItemIsSelectable));
                gate_name_item->setFlags(Qt::ItemIsEnabled);
                gate_name_item->setData(Qt::UserRole, ep_source.get_gate()->get_id());

                m_source_pins_table->setItem(index, 0, pin_name);
                m_source_pins_table->setItem(index, 1, arrow_item);
                m_source_pins_table->setItem(index, 2, gate_name_item);
                index++;
            }
        }

        m_source_pins_table->resizeColumnsToContents();
        m_source_pins_table->setFixedWidth(calculate_table_size(m_source_pins_table).width());

        // (3) update destinations section
        m_destination_pins_table->clearContents();
        m_destination_pins_button->setText("Destinations (" + QString::number(n->get_destinations().size()) + ")");
        m_destination_pins_table->setRowCount(n->get_destinations().size());
        m_destination_pins_table->setMaximumHeight(m_destination_pins_table->verticalHeader()->length());
        index = 0;
        if (!g_netlist->is_global_output_net(n))
        {
            for (const auto& ep_destination : n->get_destinations())
            {
                QTableWidgetItem* pin_name       = new QTableWidgetItem(QString::fromStdString(ep_destination.get_pin()));
                QTableWidgetItem* arrow_item     = new QTableWidgetItem(QChar(0x27a1));
                QTableWidgetItem* gate_name_item = new QTableWidgetItem(QString::fromStdString(ep_destination.get_gate()->get_name()));
                arrow_item->setForeground(QBrush(QColor(114, 140, 0), Qt::SolidPattern));
                pin_name->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
                arrow_item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
                //arrow_item->setFlags((Qt::ItemFlag)(~Qt::ItemIsSelectable));
                gate_name_item->setFlags(Qt::ItemIsEnabled);
                gate_name_item->setData(Qt::UserRole, ep_destination.get_gate()->get_id());

                m_destination_pins_table->setItem(index, 0, pin_name);
                m_destination_pins_table->setItem(index, 1, arrow_item);
                m_destination_pins_table->setItem(index, 2, gate_name_item);
                index++;
            }
        }
        m_destination_pins_table->resizeColumnsToContents();
        m_destination_pins_table->setFixedWidth(calculate_table_size(m_destination_pins_table).width());

        if (m_hide_empty_sections)
            hide_empty_sections();

        //to prevent any updating(render) erros that can occur, manually tell the tables to update
        m_general_table->update();
        m_source_pins_table->update();
        m_destination_pins_table->update();
    }

    void NetDetailsWidget::handle_net_removed(const std::shared_ptr<Net> n)
    {
        Q_UNUSED(n)
    }

    void NetDetailsWidget::handle_net_name_changed(const std::shared_ptr<Net> n)
    {
        if (m_current_id == n->get_id())
            m_name_item->setText(QString::fromStdString(n->get_name()));
    }

    void NetDetailsWidget::handle_net_source_added(const std::shared_ptr<Net> n, const u32 src_gate_id)
    {
        Q_UNUSED(src_gate_id);

        if (m_current_id == n->get_id())
            update(m_current_id);
    }

    void NetDetailsWidget::handle_net_source_removed(const std::shared_ptr<Net> n, const u32 src_gate_id)
    {
        Q_UNUSED(src_gate_id);

        if (m_current_id == n->get_id())
            update(m_current_id);
    }

    void NetDetailsWidget::handle_net_destination_added(const std::shared_ptr<Net> n, const u32 dst_gate_id)
    {
        Q_UNUSED(dst_gate_id);

        if (m_current_id == n->get_id())
            update(m_current_id);
    }

    void NetDetailsWidget::handle_net_destination_removed(const std::shared_ptr<Net> n, const u32 dst_gate_id)
    {
        Q_UNUSED(dst_gate_id);

        if (m_current_id == n->get_id())
            update(m_current_id);
    }

    void NetDetailsWidget::handle_gate_name_changed(const std::shared_ptr<Gate> g)
    {
        Q_UNUSED(g)

        if (m_current_id == 0)
            return;

        bool update_needed = false;

        //current net
        auto n = g_netlist->get_net_by_id(m_current_id);

        //check if current net is in netlist (m_current_id is unassigned if netlist details widget hasn't been shown once)
        if (!g_netlist->is_net_in_netlist(n))
            return;

        //check if renamed gate is a src of the currently shown net
        for (auto e : n->get_sources())
        {
            if (e.get_gate()->get_id() == m_current_id)
            {
                update_needed = true;
                break;
            }
        }

        //check if renamed gate is a dst of the currently shown net
        if (!update_needed)
        {
            for (auto e : n->get_destinations())
            {
                if (e.get_gate()->get_id() == m_current_id)
                {
                    update_needed = true;
                    break;
                }
            }
        }

        if (update_needed)
            update(m_current_id);
    }

    void NetDetailsWidget::handle_buttons_clicked()
    {
        QPushButton* btn = dynamic_cast<QPushButton*>(sender());
        if (!btn)
            return;

        int index = m_top_lvl_layout->indexOf(btn);
        QWidget* widget;
        widget = m_top_lvl_layout->itemAt(index + 1)->layout()->itemAt(0)->widget();

        if (!widget)
            return;

        if (widget->isHidden())
            widget->show();
        else
            widget->hide();
    }

    void NetDetailsWidget::handle_table_item_clicked(QTableWidgetItem* item)
    {
        if (item->column() != 2)
            return;

        QTableWidget* sender_table = dynamic_cast<QTableWidget*>(sender());

        SelectionRelay::subfocus focus = (sender_table == m_source_pins_table) ? SelectionRelay::subfocus::right : SelectionRelay::subfocus::left;
        auto gate_id                   = item->data(Qt::UserRole).toInt();
        auto pin                       = sender_table->item(item->row(), 0)->text().toStdString();

        auto clicked_gate = g_netlist->get_gate_by_id(gate_id);
        if (!clicked_gate)
            return;

        g_selection_relay.clear();
        g_selection_relay.m_selected_gates.insert(gate_id);
        g_selection_relay.m_focus_type = SelectionRelay::item_type::gate;
        g_selection_relay.m_focus_id   = gate_id;
        g_selection_relay.m_subfocus   = focus;

        auto pins                          = (sender_table == m_source_pins_table) ? clicked_gate->get_output_pins() : clicked_gate->get_input_pins();
        auto index                         = std::distance(pins.begin(), std::find(pins.begin(), pins.end(), pin));
        g_selection_relay.m_subfocus_index = index;

        g_selection_relay.relay_selection_changed(this);
    }

    void NetDetailsWidget::handle_general_table_menu_requeted(const QPoint& pos)
    {
        if (!m_general_table->itemAt(pos) || m_general_table->itemAt(pos)->column() != 1 || m_general_table->itemAt(pos)->row() == 1)
            return;

        auto curr_item = m_general_table->itemAt(pos);
        QMenu menu;
        QString description;
        QString python_command = "netlist.get_net_by_id(" + QString::number(m_current_id) + ").";
        QString raw_string = "", raw_desc = "";
        switch (curr_item->row())
        {
            case 0:
                python_command += "get_name()";
                description = "Extract name as python code (copy to clipboard)";
                raw_string  = m_general_table->itemAt(pos)->text();
                raw_desc    = "Extract raw name (copy to clipboard)";
                break;
            case 1:
                break;    //there is no "explicit" type
            case 2:
                python_command += "get_id()";
                description = "Extract id as python code (copy to clipboard)";
                raw_string  = m_general_table->itemAt(pos)->text();
                raw_desc    = "Ectract raw id (copy to clipboard)";
                break;
        }

        if(m_general_table->itemAt(pos)->row() == 0)
        {
            menu.addAction("Change name", [this, curr_item](){
                InputDialog ipd("Change name", "New name", curr_item->text());
                if(ipd.exec() == QDialog::Accepted)
                {
                    g_netlist->get_net_by_id(m_current_id)->set_name(ipd.text_value().toStdString());
                    update(m_current_id);
                }
            });
        }

        if (!raw_string.isEmpty())
        {
            menu.addAction(raw_desc, [raw_string]() { QApplication::clipboard()->setText(raw_string); });
        }

        menu.addAction(QIcon(":/icons/python"), description, [python_command]() { QApplication::clipboard()->setText(python_command); });

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();
    }

    void NetDetailsWidget::handle_sources_table_menu_requeted(const QPoint& pos)
    {
        if (!m_source_pins_table->itemAt(pos) || m_source_pins_table->itemAt(pos)->column() != 2)
            return;

        QMenu menu;
        menu.addAction("Jump to source gate", [this, pos]() { handle_table_item_clicked(m_source_pins_table->itemAt(pos)); });
        menu.addAction(QIcon(":/icons/python"), "Extract gate as python code (copy to clipboard)", [this, pos]() {
            QApplication::clipboard()->setText("netlist.get_gate_by_id(" + m_source_pins_table->itemAt(pos)->data(Qt::UserRole).toString() + ")");
        });

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();
    }

    void NetDetailsWidget::handle_destinations_table_menu_requeted(const QPoint& pos)
    {
        if (!m_destination_pins_table->itemAt(pos) || m_destination_pins_table->itemAt(pos)->column() != 2)
            return;

        QMenu menu;
        menu.addAction("Jump to destination gate", [this, pos]() { handle_table_item_clicked(m_destination_pins_table->itemAt(pos)); });
        menu.addAction(QIcon(":/icons/python"), "Extract gate as python code (copy to clipboard)", [this, pos]() {
            QApplication::clipboard()->setText("netlist.get_gate_by_id(" + m_destination_pins_table->itemAt(pos)->data(Qt::UserRole).toString() + ")");
        });

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();
    }

    QSize NetDetailsWidget::calculate_table_size(QTableWidget* table)
    {
        //necessary to test if the table is empty, otherwise (due to the resizeColumnsToContents function)
        //is the tables width far too big, so just return 0 as the size
        if (!table->rowCount())
            return QSize(0, 0);

        int w = table->verticalHeader()->width() + 4;    // +4 seems to be needed
        for (int i = 0; i < table->columnCount(); i++)
            w += table->columnWidth(i);    // seems to include gridline
        int h = table->horizontalHeader()->height() + 4;
        for (int i = 0; i < table->rowCount(); i++)
            h += table->rowHeight(i);
        return QSize(w + 5, h);
    }

    void NetDetailsWidget::show_all_sections()
    {
        //no need to be dynamic like in the GateDetailsWidget since there are only 2 sections that could be hidden
        m_source_pins_button->show();
        m_top_lvl_layout->itemAt(5)->spacerItem()->changeSize(0, 7, QSizePolicy::Expanding, QSizePolicy::Fixed);
        m_top_lvl_layout->itemAt(5)->spacerItem()->invalidate();
        m_destination_pins_button->show();
        m_top_lvl_layout->invalidate();
        m_top_lvl_layout->update();
    }

    void NetDetailsWidget::hide_empty_sections()
    {
        //as in show_all_sections, no need to be dynamic, only 2 possible sections that can be hidden
        if (m_source_pins_button->text().contains("(0)"))
        {
            m_source_pins_button->hide();
            m_top_lvl_layout->itemAt(5)->spacerItem()->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
            m_top_lvl_layout->itemAt(5)->spacerItem()->invalidate();
        }

        if (m_destination_pins_button->text().contains("(0)"))
            m_destination_pins_button->hide();

        m_top_lvl_layout->invalidate();
        m_top_lvl_layout->update();
    }

    void NetDetailsWidget::init_settings()
    {
        m_hide_empty_sections = g_settings_manager.get("selection_details/hide_empty_sections", false).toBool();
    }

    void NetDetailsWidget::handle_global_settings_changed(void* sender, const QString& key, const QVariant& value)
    {
        Q_UNUSED(sender)
        if (key == "selection_details/hide_empty_sections")
        {
            m_hide_empty_sections = value.toBool();
            if (!m_hide_empty_sections)
                show_all_sections();
            else
                hide_empty_sections();
        }
    }
}    // namespace hal
