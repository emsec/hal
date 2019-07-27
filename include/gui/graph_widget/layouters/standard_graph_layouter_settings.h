#ifndef STANDARD_GRAPH_LAYOUTER_SETTINGS_H
#define STANDARD_GRAPH_LAYOUTER_SETTINGS_H

#include "gui/graph_widget/layouters/graph_layouter_settings.h"

struct standard_graph_layouter_settings : public graph_layouter_settings
{
    enum type
    {
        standard,
        cone
    } type;
};

#endif // STANDARD_GRAPH_LAYOUTER_SETTINGS_H
