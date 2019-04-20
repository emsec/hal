#include "graph_widget/graph_scene_manager.h"
#include "graph_widget/graphics_items/global_graphics_net.h"
#include "graph_widget/graphics_items/graphics_gate.h"
#include "graph_widget/graphics_items/graphics_net.h"
#include "graph_widget/graphics_items/separated_graphics_net.h"
#include "graph_widget/graphics_items/standard_graphics_net.h"
#include "netlist/gate.h"
#include "netlist/net.h"

graph_scene_manager::graph_scene_manager(QObject* parent) : QObject(parent), m_scene(this)
{
    //m_scene.setBspTreeDepth(10);

    //relay connects
}

graph_scene* graph_scene_manager::get_scene()
{
    return &m_scene;
}

QMap<u32, graphics_gate*>& graph_scene_manager::get_gate_map()
{
    return m_gate_map;
}

QMap<u32, graphics_net*>& graph_scene_manager::get_net_map()
{
    return m_net_map;
}
