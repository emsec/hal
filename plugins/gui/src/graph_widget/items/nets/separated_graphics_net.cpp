#include "gui/graph_widget/items/nets/separated_graphics_net.h"

#include "hal_core/netlist/net.h"

#include "gui/graph_widget/graph_widget_constants.h"

namespace hal
{
    qreal SeparatedGraphicsNet::sAlpha;

    void SeparatedGraphicsNet::updateAlpha()
    {
        if (sLod >= graph_widget_constants::sNetFadeInLod && sLod <= graph_widget_constants::sNetFadeOutLod)
            sAlpha = (sLod - graph_widget_constants::sNetFadeInLod) / (graph_widget_constants::sNetFadeOutLod - graph_widget_constants::sNetFadeInLod);
        else
            sAlpha = 1;
    }

    SeparatedGraphicsNet::SeparatedGraphicsNet(Net* n) : GraphicsNet(n)
    {
    }

    void SeparatedGraphicsNet::finalize()
    {
        // RECT INTENTIONALLY SET SLIGHTLY TOO BIG
        mRect = mShape.boundingRect();
        mRect.adjust(-sLineWidth, -sLineWidth, sLineWidth, sLineWidth);
    }
}
