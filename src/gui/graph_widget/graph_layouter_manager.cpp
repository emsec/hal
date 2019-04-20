#include "graph_widget/graph_layouter_manager.h"

#include "graph_widget/graph_scene_manager.h"
#include "graph_widget/graphics_items/global_graphics_net.h"
#include "graph_widget/graphics_items/graph_graphics_item.h"
#include "graph_widget/layouters/graph_layouter.h"

#include "graph_widget/graphics_items/graphics_net.h"
#include "graph_widget/graphics_items/red_pin_gate.h"
#include "graph_widget/graphics_items/separated_graphics_net.h"
#include "graph_widget/graphics_items/standard_graphics_gate.h"
#include "graph_widget/graphics_items/standard_graphics_net.h"
#include "graph_widget/layouters/minimal_graph_layouter.h"
#include "graph_widget/layouters/netlist_layouter.h"
#include "graph_widget/layouters/parallel_graph_layouter.h"
#include "graph_widget/layouters/standard_graph_layouter.h"

#include "gui_globals.h"

#include <QRectF>

graph_layouter_manager::graph_layouter_manager(QObject* parent) : QObject(parent)
{
    m_layouters.append(qMakePair(new minimal_graph_layouter(), 0));
    m_layouters.append(qMakePair(new parallel_graph_layouter(), 0));
    m_layouters.append(qMakePair(new standard_graph_layouter(), 0));
    m_layouters.append(qMakePair(new netlist_layouter(), 0));
}

graph_layouter_manager& graph_layouter_manager::get_instance()
{
    static graph_layouter_manager instance;
    return instance;
}

graph_layouter_manager::~graph_layouter_manager()
{
}

graph_scene* graph_layouter_manager::subscribe(QString layouter)
{
    for (auto& pair : m_layouters)
    {
        if (pair.first->get_name() == layouter)
        {
            pair.second++;

            for (auto& inner_pair : m_scenes)
            {
                if (inner_pair.first == pair.first)
                    return inner_pair.second->get_scene();
            }

            graph_scene_manager* manager = new graph_scene_manager();
            m_scenes.append(qMakePair(pair.first, manager));

            graph_graphics_item::load_settings();
            //graphics_gate::load_settings();
            standard_graphics_gate::load_settings();
            red_pin_gate::load_settings();

            graphics_net::load_settings();
            global_graphics_net::load_settings();
            separated_graphics_net::load_settings();

            pair.first->layout(manager->get_scene());

            QRectF rect(manager->get_scene()->itemsBoundingRect());
            rect.adjust(-200, -200, 200, 200);
            manager->get_scene()->setSceneRect(rect);

            //connect scenemanager to selection relay

            connect(&g_graph_relay, &graph_relay::netlist_event, pair.first, &graph_layouter::handle_netlist_event);
            connect(&g_graph_relay, &graph_relay::gate_event, pair.first, &graph_layouter::handle_gate_event);
            connect(&g_graph_relay, &graph_relay::net_event, pair.first, &graph_layouter::handle_net_event);
            connect(&g_graph_relay, &graph_relay::module_event, pair.first, &graph_layouter::handle_module_event);
            return manager->get_scene();
        }
    }
    return nullptr;
}

void graph_layouter_manager::unsubscribe(QString layouter)
{
    for (auto& pair : m_layouters)
    {
        if (pair.first->get_name() == layouter)
        {
            if (pair.second == 0)
                return;

            pair.second--;

            if (pair.second == 0)
            {
                for (auto& inner_pair : m_scenes)
                {
                    if (inner_pair.first == pair.first)
                    {
                        pair.first->reset();
                        disconnect(&g_graph_relay, &graph_relay::netlist_event, pair.first, &graph_layouter::handle_netlist_event);
                        disconnect(&g_graph_relay, &graph_relay::gate_event, pair.first, &graph_layouter::handle_gate_event);
                        disconnect(&g_graph_relay, &graph_relay::net_event, pair.first, &graph_layouter::handle_net_event);
                        disconnect(&g_graph_relay, &graph_relay::module_event, pair.first, &graph_layouter::handle_module_event);
                        delete inner_pair.second;
                        m_scenes.removeOne(inner_pair);
                        return;
                    }
                }
            }
        }
    }
}

QStringList graph_layouter_manager::get_layouters()
{
    QStringList list;
    for (auto pair : m_layouters)
        list.append(pair.first->get_name());

    return list;
}

QString graph_layouter_manager::get_name(QString layouter)
{
    for (auto pair : m_layouters)
    {
        if (pair.first->get_name() == layouter)
            return pair.first->get_name();
    }
    return "";
}

QString graph_layouter_manager::get_description(QString layouter)
{
    for (auto pair : m_layouters)
    {
        if (pair.first->get_name() == layouter)
            return pair.first->get_description();
    }
    return "";
}
