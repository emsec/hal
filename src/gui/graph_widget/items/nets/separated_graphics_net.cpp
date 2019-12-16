#include "graph_widget/items/nets/separated_graphics_net.h"

#include "netlist/net.h"

#include "graph_widget/graph_widget_constants.h"

qreal separated_graphics_net::s_alpha;

void separated_graphics_net::update_alpha()
{
    if (s_lod >= graph_widget_constants::net_fade_in_lod && s_lod <= graph_widget_constants::net_fade_out_lod)
        s_alpha = (s_lod - graph_widget_constants::net_fade_in_lod) / (graph_widget_constants::net_fade_out_lod - graph_widget_constants::net_fade_in_lod);
    else
        s_alpha = 1;
}

separated_graphics_net::separated_graphics_net(const std::shared_ptr<const net> n) : graphics_net(n),
  m_line_style(line_style::solid),
  m_draw_output(false)
{

}
