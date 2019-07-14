#include "gui/graph_widget/layouters/graph_layouter.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/graphics_gate_factory.h"
#include "gui/graph_widget/graphics_items/global_graphics_net.h"
#include "gui/graph_widget/graphics_items/graphics_gate.h"
#include "gui/graph_widget/graphics_items/graphics_item.h"
#include "gui/graph_widget/graphics_items/graphics_net.h"
#include "gui/graph_widget/graphics_items/separated_graphics_net.h"
#include "gui/graph_widget/graphics_items/standard_graphics_net.h"
#include "gui/gui_globals.h"

graph_layouter::graph_layouter(const graph_context* const context) :
    m_scene(new graphics_scene()),
    m_context(context)
{

}

graph_layouter::~graph_layouter()
{
    delete m_scene;
}

//graphics_gate* graph_layouter::create_graphics_gate(std::shared_ptr<gate> g)
//{
//    graphics_gate* item = graphics_gate_factory::create_graphics_gate(g, 0); // USE VARIABLE TYPE

//    if (item)
//    {
//        int i = 0;
//        while (i < m_gate_vector.size())
//        {
//            if (g->get_id() < m_gate_vector.at(i).first)
//                break;

//            i++;
//        }
//        m_gate_vector.insert(i, QPair<u32, graphics_gate*>(g->get_id(), item));
//    }
//    return item;
//}

//standard_graphics_net* graph_layouter::create_standard_graphics_net(std::shared_ptr<net> n)
//{
//    standard_graphics_net* item = new standard_graphics_net(n);

//    if (item)
//        sort_into_net_vector(item);
//    return item;
//}

//global_graphics_net* graph_layouter::create_global_graphics_net(std::shared_ptr<net> n)
//{
//    global_graphics_net* item = new global_graphics_net(n);

//    if (item)
//        sort_into_net_vector(item);
//    return item;
//}

//separated_graphics_net* graph_layouter::create_separated_graphics_net(std::shared_ptr<net> n)
//{
//    separated_graphics_net* item = new separated_graphics_net(QString::fromStdString(n->get_name()), n);

//    if (item)
//        sort_into_net_vector(item);
//    return item;
//}

//const QString& graph_layouter::name() const
//{
//    return m_name;
//}

//const QString& graph_layouter::description() const
//{
//    return m_description;
//}

graphics_scene* graph_layouter::scene() const
{
    return m_scene;
}

//void graph_layouter::sort_into_net_vector(graphics_net* item)
//{
//    int i = 0;
//    while (i < m_net_vector.size())
//    {
//        if (item->get_net()->get_id() < m_net_vector.at(i).first)
//            break;

//        ++i;
//    }
//    m_net_vector.insert(i, QPair<u32, graphics_net*>(item->get_net()->get_id(), item));
//}
