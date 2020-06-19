#include "selection_details_widget/gate_details_widget.h"

#include "netlist/gate.h"
#include "netlist/net.h"

#include "gui_globals.h"
#include "gui_utils/geometry.h"

#include "graph_widget/graph_navigation_widget.h"
#include "netlist/module.h"

#include <QApplication>
#include <QCursor>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QResizeEvent>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollBar>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QApplication> //to extract the stylesheet of the main app.
#include <QMenu>
#include <QIcon>
#include <QClipboard>

namespace hal
{
    gate_details_widget::gate_details_widget(QWidget* parent) : QWidget(parent)
    {
        //NEW
        // general initializations
        init_settings();
        m_current_id = 0;
        m_key_font = QFont("Iosevka");
        m_key_font.setBold(true);
        m_key_font.setPixelSize(13);

        //this line throws a warning that there is already an existing layout, yet there is no layout set and
        //even after calling delete layout(); and then setting the layout, the warning continues
        m_content_layout = new QVBoxLayout(this);
        m_scroll_area = new QScrollArea(this);
        m_top_lvl_container = new QWidget(this);
        m_top_lvl_layout = new QVBoxLayout(m_top_lvl_container);
        m_top_lvl_container->setLayout(m_top_lvl_layout);;
        m_scroll_area->setWidget(m_top_lvl_container);
        m_scroll_area->setWidgetResizable(true);

        //layout customization
        m_content_layout->setContentsMargins(0,0,0,0);
        m_content_layout->setSpacing(0);
        m_top_lvl_layout->setContentsMargins(0,0,0,0);
        m_top_lvl_layout->setSpacing(0);

        //container-layouts to add spacing Widgets (gt = general table, op = output pins, etc)
        QHBoxLayout *intermediate_layout_gt = new QHBoxLayout();
        intermediate_layout_gt->setContentsMargins(3,3,0,0);
        intermediate_layout_gt->setSpacing(0);
        QHBoxLayout *intermediate_layout_ip = new QHBoxLayout();
        intermediate_layout_ip->setContentsMargins(3,3,0,0);
        intermediate_layout_ip->setSpacing(10);
        QHBoxLayout *intermediate_layout_op = new QHBoxLayout();
        intermediate_layout_op->setContentsMargins(3,3,0,0);
        intermediate_layout_op->setSpacing(0);
        QHBoxLayout *intermediate_layout_df = new QHBoxLayout();
        intermediate_layout_df->setContentsMargins(3,3,0,0);
        intermediate_layout_df->setSpacing(0);

        // buttons
        m_general_info_button = new QPushButton("General Information", this);
        m_general_info_button->setEnabled(false);
        m_input_pins_button = new QPushButton("Input Pins", this);
        m_output_pins_button = new QPushButton("Output Pins", this);
        m_data_fields_button = new QPushButton("Data Fields", this);
        m_boolean_functions_button = new QPushButton("Boolean Functions", this);

        // table initializations (section 1-4)
        m_general_table = new QTableWidget(4,2,this);
        m_input_pins_table = new QTableWidget(0,3, this);
        m_output_pins_table = new QTableWidget(0,3, this);
        m_data_fields_table = new QTableWidget(0, 2, this);

        //shared stlye options (every option is applied to each table)
        QList<QTableWidget*> tmp;
        tmp << m_general_table << m_input_pins_table << m_output_pins_table << m_data_fields_table;
        for(auto & table : tmp)
        {
            //table->horizontalHeader()->setStretchLastSection(true);
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

        //customize general section by adding the fixed iitems
        QList<QTableWidgetItem*> tmp_general_info_list = {new QTableWidgetItem("Name:"), new QTableWidgetItem("Type:"),
                                                          new QTableWidgetItem("ID:"), new QTableWidgetItem("Module:")};
        for(int i = 0; i < tmp_general_info_list.size(); i++)
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
        m_type_item->setFlags(Qt::ItemIsEnabled);
        m_general_table->setItem(1, 1, m_type_item);

        m_id_item = new QTableWidgetItem();
        m_id_item->setFlags(Qt::ItemIsEnabled);
        m_general_table->setItem(2, 1, m_id_item);

        m_module_item = new QTableWidgetItem();
        m_module_item->setFlags(Qt::ItemIsEnabled);
        m_general_table->setItem(3, 1, m_module_item);


        //(5) Boolean Function section
        m_boolean_functions_container = new QWidget(this);
        m_boolean_functions_container_layout = new QVBoxLayout(this);
        m_boolean_functions_container_layout->setContentsMargins(6,5,0,0);
        m_boolean_functions_container_layout->setSpacing(0);
        m_boolean_functions_container->setLayout(m_boolean_functions_container_layout);

        //adding things to intermediate layout (the one thats neccessary for the left spacing)
        intermediate_layout_gt->addWidget(m_general_table);
        intermediate_layout_gt->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Fixed));
        intermediate_layout_ip->addWidget(m_input_pins_table);
        intermediate_layout_ip->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Fixed));
        intermediate_layout_op->addWidget(m_output_pins_table);
        intermediate_layout_op->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Fixed));
        intermediate_layout_df->addWidget(m_data_fields_table);
        intermediate_layout_df->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Fixed));

        //adding things to the main layout
        m_top_lvl_layout->addWidget(m_general_info_button);
        m_top_lvl_layout->addLayout(intermediate_layout_gt);
        m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 7, QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_top_lvl_layout->addWidget(m_input_pins_button);
        m_top_lvl_layout->addLayout(intermediate_layout_ip);
        m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 7, QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_top_lvl_layout->addWidget(m_output_pins_button);
        m_top_lvl_layout->addLayout(intermediate_layout_op);
        m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 7, QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_top_lvl_layout->addWidget(m_data_fields_button);
        m_top_lvl_layout->addLayout(intermediate_layout_df);
        m_top_lvl_layout->addSpacerItem(new QSpacerItem(0, 7, QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_top_lvl_layout->addWidget(m_boolean_functions_button);
        m_top_lvl_layout->addWidget(m_boolean_functions_container);

        //necessary to add at the end
        m_top_lvl_layout->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Expanding));
        m_content_layout->addWidget(m_scroll_area);

        //setup the navigation_table ("activated" by clicking on an input / output pin in the 2 tables)
        //delete the table manually so its not necessarry to add a property for the stylesheet(otherwise this table is styled like the others)
        m_navigation_table = new graph_navigation_widget();
        m_navigation_table->setWindowFlags(Qt::CustomizeWindowHint);
        m_navigation_table->hide_when_focus_lost(true);
        m_navigation_table->hide();
        connect(m_navigation_table, &graph_navigation_widget::navigation_requested, this, &gate_details_widget::handle_navigation_jump_requested);

        //some connections (maybe connect to simple toggle_hide_show functiom_boolean_functions_container_layoutn of widgets)
        connect(m_general_info_button, &QPushButton::clicked, this, &gate_details_widget::handle_buttons_clicked);
        connect(m_input_pins_button, &QPushButton::clicked, this, &gate_details_widget::handle_buttons_clicked);
        connect(m_output_pins_button, &QPushButton::clicked, this, &gate_details_widget::handle_buttons_clicked);
        connect(m_data_fields_button, &QPushButton::clicked, this, &gate_details_widget::handle_buttons_clicked);
        connect(m_boolean_functions_button, &QPushButton::clicked, this, &gate_details_widget::handle_buttons_clicked);

        connect(m_input_pins_table, &QTableWidget::itemDoubleClicked, this, &gate_details_widget::handle_input_pin_item_clicked);
        connect(m_output_pins_table, &QTableWidget::itemDoubleClicked, this, &gate_details_widget::handle_output_pin_item_clicked);
        connect(m_general_table, &QTableWidget::itemDoubleClicked, this, &gate_details_widget::handle_general_table_item_clicked);

        //context menu connects
        connect(m_general_table, &QTableWidget::customContextMenuRequested, this, &gate_details_widget::handle_general_table_menu_requested);
        connect(m_input_pins_table, &QTableWidget::customContextMenuRequested, this, &gate_details_widget::handle_input_pin_table_menu_requested);
        connect(m_output_pins_table, &QTableWidget::customContextMenuRequested, this, &gate_details_widget::handle_output_pin_table_menu_requested);
        connect(m_data_fields_table, &QTableWidget::customContextMenuRequested, this, &gate_details_widget::handle_data_table_menu_requested);

        //settings
        connect(&g_settings_relay, &settings_relay::setting_changed, this, &gate_details_widget::handle_global_settings_changed);

        //install eventfilers
        m_general_table->viewport()->setMouseTracking(true);
        m_general_table->viewport()->installEventFilter(this);
        m_input_pins_table->viewport()->setMouseTracking(true);
        m_input_pins_table->viewport()->installEventFilter(this);
        m_output_pins_table->viewport()->setMouseTracking(true);
        m_output_pins_table->viewport()->installEventFilter(this);

        //extract the width of the scrollbar out of the stylesheet to fix a scrollbar related bug
        QString main_stylesheet = qApp->styleSheet();
        main_stylesheet.replace("\n", ""); //remove newlines so the regex is a bit easier
        QRegularExpression re(".+?QScrollBar:vertical ?{[^}]+?(?: *width *?|; *width *?): *([0-9]*)[^;]*");
        QRegularExpressionMatch ma = re.match(main_stylesheet);
        m_scrollbar_width = (ma.hasMatch()) ? ma.captured(1).toInt() : 0;

        m_util_list << m_input_pins_button  << m_output_pins_button  << m_data_fields_button << m_boolean_functions_button;
    }

    gate_details_widget::~gate_details_widget()
    {
        delete m_navigation_table;
    }

    void gate_details_widget::handle_gate_name_changed(std::shared_ptr<Gate> gate)
    {
        if (m_current_id == gate->get_id())
            update(m_current_id);
    }

    void gate_details_widget::handle_gate_removed(std::shared_ptr<Gate> gate)
    {
        if (m_current_id == gate->get_id())
        {
            m_general_table->setHidden(true);
            m_scroll_area->setHidden(true);
        }
    }

    void gate_details_widget::handle_net_name_changed(std::shared_ptr<Net> net)
    {
        bool update_needed = false;

        //check if currently shown gate is a src of renamed net
        for (auto& e : net->get_sources())
        {
            if (m_current_id == e.get_gate()->get_id())
            {
                update_needed = true;
                break;
            }
        }

        //check if currently shown gate is a dst of renamed net
        if (!update_needed)
        {
            for (auto& e : net->get_destinations())
            {
                if (m_current_id == e.get_gate()->get_id())
                {
                    update_needed = true;
                    break;
                }
            }
        }

        if (update_needed)
            update(m_current_id);
    }

    void gate_details_widget::handle_net_source_added(std::shared_ptr<Net> net, const u32 src_gate_id)
    {
        Q_UNUSED(net);
        if (m_current_id == src_gate_id)
            update(m_current_id);
    }

    void gate_details_widget::handle_net_source_removed(std::shared_ptr<Net> net, const u32 src_gate_id)
    {
        Q_UNUSED(net);
        if (m_current_id == src_gate_id)
            update(m_current_id);
    }

    void gate_details_widget::handle_net_destination_added(std::shared_ptr<Net> net, const u32 dst_gate_id)
    {
        Q_UNUSED(net);
        if (m_current_id == dst_gate_id)
            update(m_current_id);
    }

    void gate_details_widget::handle_net_destination_removed(std::shared_ptr<Net> net, const u32 dst_gate_id)
    {
        Q_UNUSED(net);
        if (m_current_id == dst_gate_id)
            update(m_current_id);
    }

    void gate_details_widget::handle_buttons_clicked()
    {
        //function that (perhaps) is changed by a toggle-slot of the widget
        QPushButton* btn = dynamic_cast<QPushButton*>(sender());
        if(!btn)
            return;

        int index = m_top_lvl_layout->indexOf(btn);
        QWidget* widget;

        if(btn != m_boolean_functions_button)
            widget = m_top_lvl_layout->itemAt(index+1)->layout()->itemAt(0)->widget();
        else
            widget = m_top_lvl_layout->itemAt(index+1)->widget();

        if(!widget)
            return;

        if(widget->isHidden())
            widget->show();
        else
            widget->hide();
    }

    void gate_details_widget::handle_input_pin_item_clicked(const QTableWidgetItem *item)
    {
        if(item->column() != 2)
            return;

        int net_id = item->data(Qt::UserRole).toInt();

        auto clicked_net = g_netlist->get_net_by_id(net_id);

        if(!clicked_net)
            return;

        auto sources = clicked_net->get_sources();

        if(sources.empty() || clicked_net->is_global_input_net())
        {
            g_selection_relay.clear();
            g_selection_relay.m_selected_nets.insert(net_id);
            g_selection_relay.relay_selection_changed(this);
        }
        else if(sources.size() == 1)
        {
            auto ep = *sources.begin();
            g_selection_relay.clear();
            g_selection_relay.m_selected_gates.insert(ep.get_gate()->get_id());
            g_selection_relay.m_focus_type = selection_relay::item_type::gate;
            g_selection_relay.m_focus_id   = ep.get_gate()->get_id();
            g_selection_relay.m_subfocus   = selection_relay::subfocus::right;

            auto pins                          = ep.get_gate()->get_output_pins();
            auto index                         = std::distance(pins.begin(), std::find(pins.begin(), pins.end(), ep.get_pin()));
            g_selection_relay.m_subfocus_index = index;

            update(ep.get_gate()->get_id());
            g_selection_relay.relay_selection_changed(this);
        }
        else
        {
            m_navigation_table->setup(hal::node{hal::node_type::gate, 0}, clicked_net, false);
            m_navigation_table->move(QCursor::pos());
            m_navigation_table->show();
            m_navigation_table->setFocus();
        }
    }

    void gate_details_widget::handle_output_pin_item_clicked(const QTableWidgetItem *item)
    {
        if(item->column() != 2)
            return;

        int net_id = item->data(Qt::UserRole).toInt();
        std::shared_ptr<Net> clicked_net = g_netlist->get_net_by_id(net_id);

        if(!clicked_net)
            return;

        auto destinations = clicked_net->get_destinations();
        if(destinations.empty() || clicked_net->is_global_output_net())
        {
            g_selection_relay.clear();
            g_selection_relay.m_selected_nets.insert(net_id);
            g_selection_relay.relay_selection_changed(this);
        }
        else if (destinations.size() == 1)
        {
            auto ep = *destinations.begin();
            g_selection_relay.clear();
            g_selection_relay.m_selected_gates.insert(ep.get_gate()->get_id());
            g_selection_relay.m_focus_type = selection_relay::item_type::gate;
            g_selection_relay.m_focus_id   = ep.get_gate()->get_id();
            g_selection_relay.m_subfocus   = selection_relay::subfocus::left;

            auto pins                          = ep.get_gate()->get_input_pins();
            auto index                         = std::distance(pins.begin(), std::find(pins.begin(), pins.end(), ep.get_pin()));
            g_selection_relay.m_subfocus_index = index;

            update(ep.get_gate()->get_id());
            g_selection_relay.relay_selection_changed(this);
        }
        else
        {
            m_navigation_table->setup(hal::node{hal::node_type::gate, 0}, clicked_net, true);
            m_navigation_table->move(QCursor::pos());
            m_navigation_table->show();
            m_navigation_table->setFocus();
        }
    }

    void gate_details_widget::handle_input_pin_table_menu_requested(const QPoint &pos)
    {
        if(m_input_pins_table->itemAt(pos)->column() != 2)
            return;

        QMenu menu;
        menu.addAction("Jump to source gate", [this, pos](){
            handle_input_pin_item_clicked(m_input_pins_table->itemAt(pos));
        });

        menu.addAction(QIcon(":/icons/python"), "Extract net as python code (copy to clipboard)",[this, pos](){
            QApplication::clipboard()->setText("netlist.get_net_by_id(" + m_input_pins_table->itemAt(pos)->data(Qt::UserRole).toString() + ")");
        });

        menu.addAction(QIcon(":/icons/python"), "Extract sources as python code (copy to clipboard)",[this, pos](){
            QApplication::clipboard()->setText("netlist.get_net_by_id(" + m_input_pins_table->itemAt(pos)->data(Qt::UserRole).toString() + ").get_sources()" );
        });

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();

    }

    void gate_details_widget::handle_output_pin_table_menu_requested(const QPoint &pos)
    {
        if(m_output_pins_table->itemAt(pos)->column() != 2)
            return;

        QMenu menu;
        menu.addAction("Jump to destination gate", [this, pos](){
            handle_output_pin_item_clicked(m_output_pins_table->itemAt(pos));
        });

        menu.addAction(QIcon(":/icons/python"), "Extract net as python code (copy to clipboard)",[this, pos](){
            QApplication::clipboard()->setText("netlist.get_net_by_id(" + m_output_pins_table->itemAt(pos)->data(Qt::UserRole).toString() + ")");
        });

        menu.addAction(QIcon(":/icons/python"), "Extract destinations as python code (copy to clipboard)",[this, pos](){
            QApplication::clipboard()->setText("netlist.get_net_by_id(" + m_output_pins_table->itemAt(pos)->data(Qt::UserRole).toString() + ").get_destinations()" );
        });

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();

    }

    void gate_details_widget::handle_data_table_menu_requested(const QPoint &pos)
    {
        if(m_data_fields_table->itemAt(pos)->column() != 1)
            return;

        QMenu menu;
        menu.addAction(QIcon(":/icons/python"), "Exctract data as python code (copy to clipboard)", [this, pos](){
           int row = m_data_fields_table->itemAt(pos)->row();
           QString key = m_data_fields_table->item(row, 0)->text().left(m_data_fields_table->item(row, 0)->text().length()-1);
           QApplication::clipboard()->setText("netlist.get_gate_by_id(" + QString::number(m_current_id) + ").data[(\"" + m_data_fields_table->item(row, 0)->data(Qt::UserRole).toString() + "\", \"" + key + "\")]");//(’generic’, ’data’)
        });

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();

    }
    void gate_details_widget::handle_general_table_menu_requested(const QPoint &pos)
    {
        if(m_general_table->itemAt(pos)->column() != 1)
            return;

        QMenu menu;
        QString description;
        QString python_command = "netlist.get_gate_by_id(" + QString::number(m_current_id) + ").";
        switch(m_general_table->itemAt(pos)->row())
        {
            case 0: python_command += "get_name()"; description = "Extract name as python code (copy to clipboard)"; break;
            case 1: python_command += "get_type()"; description = "Extract type as python code (copy to clipboard)"; break;
            case 2: python_command += "get_id()"; description = "Extract id as python code (copy to clipboard)"; break;
            case 3: python_command += "get_module()"; description = "Extract module as python code (copy to clipboard)"; break;
        }
        menu.addAction(QIcon(":/icons/python"), description, [python_command](){
            QApplication::clipboard()->setText(python_command);
        });

        menu.move(dynamic_cast<QWidget*>(sender())->mapToGlobal(pos));
        menu.exec();
    }

    QSize gate_details_widget::calculate_table_size(QTableWidget *table)
    {
        //necessary to test if the table is empty, otherwise (due to the resizeColumnsToContents function)
        //is the tables width far too big, so just return 0 as the size
        if(!table->rowCount())
            return QSize(0,0);

        int w = table->verticalHeader()->width() + 4; // +4 seems to be needed
        for (int i = 0; i < table->columnCount(); i++)
           w += table->columnWidth(i); // seems to include gridline
        int h = table->horizontalHeader()->height() + 4;
        for (int i = 0; i < table->rowCount(); i++)
           h += table->rowHeight(i);
        return QSize(w+5, h);

    }

    void gate_details_widget::show_all_sections()
    {
        for(auto section_btn : m_util_list)
        {
            int index = m_top_lvl_layout->indexOf(section_btn);
            section_btn->show();
            if(section_btn != m_boolean_functions_button)
            {
                m_top_lvl_layout->itemAt(index+2)->spacerItem()->changeSize(0,7,QSizePolicy::Expanding, QSizePolicy::Fixed);
                m_top_lvl_layout->itemAt(index+2)->spacerItem()->invalidate();
            }
        }
        m_top_lvl_layout->invalidate();
        m_top_lvl_layout->update();
    }

    void gate_details_widget::hide_empty_sections()
    {
        //hide when necessary
        for(auto section_btn : m_util_list)
        {
            QPushButton* curr_btn = dynamic_cast<QPushButton*>(section_btn);
            if(curr_btn->text().contains("(0)"))
            {
                curr_btn->hide();
                int index = m_top_lvl_layout->indexOf(section_btn);
                if(section_btn != m_boolean_functions_button)
                {
                    m_top_lvl_layout->itemAt(index+2)->spacerItem()->changeSize(0,0,QSizePolicy::Fixed, QSizePolicy::Fixed);
                    m_top_lvl_layout->itemAt(index+2)->spacerItem()->invalidate();
                }
            }
        }
        m_top_lvl_layout->invalidate();
        m_top_lvl_layout->update();
    }

    void gate_details_widget::init_settings()
    {
        m_hide_empty_sections = g_settings_manager.get("selection_details/hide_empty_sections", false).toBool();
    }

    void gate_details_widget::handle_global_settings_changed(void* sender, const QString& key, const QVariant& value)
    {
        Q_UNUSED(sender)
        if(key == "selection_details/hide_empty_sections")
        {
            m_hide_empty_sections = value.toBool();
            if(!m_hide_empty_sections)
                show_all_sections();
            else
                hide_empty_sections();
        }
    }

    void gate_details_widget::handle_module_removed(std::shared_ptr<Module> module)
    {
        if (m_current_id == 0)
            return;
        auto g = g_netlist->get_gate_by_id(m_current_id);

        if (module->contains_gate(g))
        {
            update(m_current_id);
        }
    }

    void gate_details_widget::handle_module_name_changed(std::shared_ptr<Module> module)
    {
        if (m_current_id == 0)
            return;
        auto g = g_netlist->get_gate_by_id(m_current_id);

        if (module->contains_gate(g))
        {
            update(m_current_id);
        }
    }

    void gate_details_widget::handle_module_gate_assigned(std::shared_ptr<Module> module, u32 associated_data)
    {
        Q_UNUSED(module);
        if (m_current_id == associated_data)
        {
            update(m_current_id);
        }
    }

    void gate_details_widget::handle_module_gate_removed(std::shared_ptr<Module> module, u32 associated_data)
    {
        Q_UNUSED(module);
        if (!g_netlist->is_gate_in_netlist(g_netlist->get_gate_by_id(associated_data)))
            return;

        if (m_current_id == associated_data)
        {
            update(m_current_id);
        }
    }

    void gate_details_widget::resizeEvent(QResizeEvent* event)
    {
        //2 is needed because just the scrollbarwitdth is not enough (does not include its border?)
        m_boolean_functions_container->setFixedWidth(event->size().width() - m_scrollbar_width-2);
    }

    bool gate_details_widget::eventFilter(QObject *watched, QEvent *event)
    {
        if((watched == m_input_pins_table->viewport() || watched == m_output_pins_table->viewport()) && event->type() == QEvent::MouseMove)
        {
            //need to determine which of the tables is the "owner" of the viewport
            QTableWidget* table = (watched == m_input_pins_table->viewport()) ? m_input_pins_table : m_output_pins_table;
            QMouseEvent* ev = dynamic_cast<QMouseEvent*>(event);
            QTableWidgetItem* item = table->itemAt(ev->pos());
            if(item)
            {
                if(item->column() == 2)
                    setCursor(QCursor(Qt::PointingHandCursor));
                else
                    setCursor(QCursor(Qt::ArrowCursor));
            }
            else
                setCursor(QCursor(Qt::ArrowCursor));

        }
        if(watched == m_general_table->viewport() && event->type() == QEvent::MouseMove)
        {
            QTableWidgetItem* item = m_general_table->itemAt(dynamic_cast<QMouseEvent*>(event)->pos());
            if(item == m_module_item)
                setCursor(QCursor(Qt::PointingHandCursor));
            else
                setCursor(QCursor(Qt::ArrowCursor));
        }

        //restore default cursor when leaving any watched widget (maybe save cursor before entering?)
        if(event->type() == QEvent::Leave)
            setCursor(QCursor(Qt::ArrowCursor));

        return false;
    }

    void gate_details_widget::update(const u32 gate_id)
    {
        auto g = g_netlist->get_gate_by_id(gate_id);
        m_current_id = gate_id;

        if(!g || m_current_id == 0)
            return;

        show_all_sections();

        //update (1)general info section
        m_name_item->setText(QString::fromStdString(g->get_name()));
        m_type_item->setText(QString::fromStdString(g->get_type()->get_name()));
        m_id_item->setText(QString::number(m_current_id));

        QString module_text = "";
        for (const auto sub : g_netlist->get_modules())
        {
            if (sub->contains_gate(g))
            {
                module_text = QString::fromStdString(sub->get_name()) + "[" + QString::number(sub->get_id()) + "]";
                m_module_item->setData(Qt::UserRole, sub->get_id());
            }
        }
        m_module_item->setText(module_text);
        m_general_table->resizeColumnsToContents();
        m_general_table->setFixedWidth(calculate_table_size(m_general_table).width());

        //update (2)input-pin section
        m_input_pins_table->clearContents();
        m_input_pins_button->setText(QString::fromStdString("Input Pins (") + QString::number(g->get_input_pins().size()) + QString::fromStdString(")"));
        m_input_pins_table->setRowCount(g->get_input_pins().size());
        m_input_pins_table->setMaximumHeight(m_input_pins_table->verticalHeader()->length());
        int index = 0;
        for(const auto &pin : g->get_input_pins())
        {
            QTableWidgetItem* pin_name = new QTableWidgetItem(QString::fromStdString(pin));
            QTableWidgetItem* arrow_item = new QTableWidgetItem(QChar(0x2b05));
            QTableWidgetItem* net_item = new QTableWidgetItem();
            arrow_item->setForeground(QBrush(QColor(114, 140, 0), Qt::SolidPattern));
            //pin_name->setFlags(Qt::ItemIsEnabled);
            pin_name->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            arrow_item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            //arrow_item->setFlags((Qt::ItemFlag)(~Qt::ItemIsSelectable));
            net_item->setFlags(Qt::ItemIsEnabled);

            auto input_net = g_netlist->get_gate_by_id(gate_id)->get_fan_in_net(pin);
            if(input_net)
            {
                net_item->setText(QString::fromStdString(input_net->get_name()));
                net_item->setData(Qt::UserRole, input_net->get_id());
            }
            else
                net_item->setText("unconnected");

            m_input_pins_table->setItem(index, 0, pin_name);
            m_input_pins_table->setItem(index, 1, arrow_item);
            m_input_pins_table->setItem(index, 2, net_item);
            index++;
        }
        m_input_pins_table->resizeColumnsToContents();
        m_input_pins_table->setFixedWidth(calculate_table_size(m_input_pins_table).width());

        //update(3) output pins section
        m_output_pins_table->clearContents();
        m_output_pins_button->setText(QString::fromStdString("Output Pins (") + QString::number(g->get_output_pins().size()) + QString::fromStdString(")"));
        m_output_pins_table->setRowCount(g->get_output_pins().size());
        m_output_pins_table->setMaximumHeight(m_output_pins_table->verticalHeader()->length());
        index = 0;
        for(const auto &pin : g->get_output_pins())
        {
            QTableWidgetItem* pin_name = new QTableWidgetItem(QString::fromStdString(pin));
            QTableWidgetItem* arrow_item = new QTableWidgetItem(QChar(0x27a1));
            QTableWidgetItem* net_item = new QTableWidgetItem();
            arrow_item->setForeground(QBrush(QColor(114, 140, 0), Qt::SolidPattern));//stylesheet?
            //pin_name->setFlags(Qt::ItemIsEnabled);
            pin_name->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            //arrow_item->setFlags(Qt::ItemIsEnabled);
            arrow_item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            net_item->setFlags(Qt::ItemIsEnabled);

            auto output_net = g_netlist->get_gate_by_id(gate_id)->get_fan_out_net(pin);
            if(output_net)
            {
                net_item->setText(QString::fromStdString(output_net->get_name()));
                net_item->setData(Qt::UserRole, output_net->get_id());
            }
            else
                net_item->setText("unconnected");

            m_output_pins_table->setItem(index, 0, pin_name);
            m_output_pins_table->setItem(index, 1, arrow_item);
            m_output_pins_table->setItem(index, 2, net_item);
            index++;
        }
        m_output_pins_table->resizeColumnsToContents();
        m_output_pins_table->setFixedWidth(calculate_table_size(m_output_pins_table).width());

        //update(4) data fields section
        m_data_fields_button->setText(QString::fromStdString("Data Fields (") + QString::number(g->get_data().size()) + QString::fromStdString(")"));
        m_data_fields_table->clearContents();
        m_data_fields_table->setRowCount(g->get_data().size());
        m_data_fields_table->setMaximumHeight(m_data_fields_table->verticalHeader()->length());
        index = 0;
        for(const auto& [key, value] : g->get_data())
        {
            QTableWidgetItem* key_item = new QTableWidgetItem(QString::fromStdString(std::get<1>(key)) + QString(":"));
            key_item->setFont(m_key_font);
            key_item->setFlags((Qt::ItemFlag)~Qt::ItemIsEnabled);
            key_item->setData(Qt::UserRole, QString::fromStdString(std::get<0>(key)));
            QTableWidgetItem* value_item = new QTableWidgetItem(QString::fromStdString(std::get<1>(value)));
            value_item->setFlags(Qt::ItemIsEnabled);

            m_data_fields_table->setItem(index, 0, key_item);
            m_data_fields_table->setItem(index, 1, value_item);
            index++;
        }
        m_data_fields_table->resizeColumnsToContents();
        m_data_fields_table->setFixedWidth(calculate_table_size(m_data_fields_table).width());


        //update(5) boolean functions section
        //clear container layout
        while(m_boolean_functions_container_layout->itemAt(0) != 0)
        {
            QLayoutItem* i = m_boolean_functions_container_layout->takeAt(0);
            delete i->widget();
            delete i;
        }

        m_boolean_functions_button->setText(QString("Boolean Functions (") + QString::number(g->get_boolean_functions().size()) + QString(")"));
        QFrame* last_line = nullptr; //unexpected behaviour below otherwise
        for(const auto& it : g->get_boolean_functions())
        {
            QLabel* fnct = new QLabel(QString::fromStdString(it.first) + " = " + QString::fromStdString(it.second.to_string()));
            fnct->setWordWrap(true);
            m_boolean_functions_container_layout->addWidget(fnct);
            QFrame* line = new QFrame;
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Sunken);
            //to outsource this line into the stylesheet, you need to make a new class that inherits from QFrame
            //and style that class, propertys and the normal way does not work (other tables are also affected)
            line->setStyleSheet("QFrame{background-color: gray;}");
            last_line = line;
            m_boolean_functions_container_layout->addWidget(line);
        }

        if(last_line){
            m_boolean_functions_container_layout->removeWidget(last_line);
            delete last_line;
        }

        if(m_hide_empty_sections)
            hide_empty_sections();

        //to prevent any updating(render) errors that can occur, manually tell the tables to update
        m_general_table->update();
        m_input_pins_table->update();
        m_output_pins_table->update();
        m_data_fields_table->update();
    }

    void gate_details_widget::handle_navigation_jump_requested(const hal::node origin, const u32 via_net, const QSet<u32>& to_gates)
    {
        Q_UNUSED(origin);

        auto n = g_netlist->get_net_by_id(via_net);

        if (to_gates.isEmpty() || !n)
            return;
        for (u32 id : to_gates)
        {
            if (!g_netlist->get_gate_by_id(id))
                return;
        }

        m_navigation_table->hide();
        g_selection_relay.clear();
        g_selection_relay.m_selected_gates = to_gates;
        if (to_gates.size() == 1)
        {
            g_selection_relay.m_focus_type = selection_relay::item_type::gate;
            auto g                         = g_netlist->get_gate_by_id(*to_gates.constBegin());
            g_selection_relay.m_focus_id   = g->get_id();
            g_selection_relay.m_subfocus   = selection_relay::subfocus::left;

            u32 index_cnt = 0;
            for (const auto& pin : g->get_input_pins())
            {
                if (g->get_fan_in_net(pin) == n)
                {
                    g_selection_relay.m_subfocus_index = index_cnt;
                    break;
                }
                index_cnt++;
            }

            g_selection_relay.relay_selection_changed(this);
        }
        m_navigation_table->hide();

        // TODO ensure gates visible in graph
    }

    void gate_details_widget::handle_general_table_item_clicked(const QTableWidgetItem *item)
    {
        //cant get the item from the index (static_cast<QTableWidgetItem*>(index.internalPointer()) fails),
        //so ask the item QTableWidgetItem directly
        if (item->row() == m_module_item->row() && item->column() == m_module_item->column())
        {
            g_selection_relay.clear();
            g_selection_relay.m_selected_modules.insert(m_module_item->data(Qt::UserRole).toInt());
            g_selection_relay.relay_selection_changed(this);
        }
    }
}
