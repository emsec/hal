#include "graph_widget/items/nets/separated_graphics_net.h"

#include "netlist/net.h"

#include "graph_widget/graph_widget_constants.h"

namespace hal
{
    qreal SeparatedGraphicsNet::s_alpha;

    void SeparatedGraphicsNet::update_alpha()
    {
        if (s_lod >= graph_widget_constants::net_fade_in_lod && s_lod <= graph_widget_constants::net_fade_out_lod)
            s_alpha = (s_lod - graph_widget_constants::net_fade_in_lod) / (graph_widget_constants::net_fade_out_lod - graph_widget_constants::net_fade_in_lod);
        else
            s_alpha = 1;
    }

    SeparatedGraphicsNet::SeparatedGraphicsNet(const std::shared_ptr<const Net> n) : GraphicsNet(n)
    {
    }

    void SeparatedGraphicsNet::finalize()
    {
        // RECT INTENTIONALLY SET SLIGHTLY TOO BIG
        m_rect = m_shape.boundingRect();
        m_rect.adjust(-s_line_width, -s_line_width, s_line_width, s_line_width);
    }
}
