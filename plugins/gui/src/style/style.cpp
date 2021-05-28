#include "gui/style/style.h"

#include "hal_core/utilities/log.h"

#include "gui/graph_widget/items/graphics_item.h"
#include "gui/graph_widget/items/nets/graphics_net.h"
#include "gui/graph_widget/items/nodes/gates/standard_graphics_gate.h"
#include "gui/graph_widget/items/nodes/modules/standard_graphics_module.h"
#include "gui/graph_widget/items/nets/arrow_separated_net.h"
#include "gui/graph_widget/items/nets/circle_separated_net.h"
#include "gui/graph_widget/items/nets/labeled_separated_net.h"
#include "gui/graph_widget/items/nets/separated_graphics_net.h"
#include "gui/graph_widget/items/nets/standard_graphics_net.h"
#include "gui/graph_widget/items/utility_items/node_drag_shadow.h"
#include "gui/gui_globals.h"
#include "gui/svg_icon_engine/svg_icon_engine.h"

#include <QApplication>
#include <QFile>
#include <QHash>
#include <QRegularExpression>
#include <QTextStream>

namespace hal
{
    namespace style
    {
        void debugUpdate()
        {
            GraphicsItem::loadSettings();
            StandardGraphicsModule::loadSettings();
            StandardGraphicsGate::loadSettings();
            GraphicsNet::loadSettings();
            StandardGraphicsNet::loadSettings();
            SeparatedGraphicsNet::loadSettings();
            ArrowSeparatedNet::loadSettings();
            CircleSeparatedNet::loadSettings();
            LabeledSeparatedNet::loadSettings();
            NodeDragShadow::loadSettings();
        }

    } // namespace style
}
