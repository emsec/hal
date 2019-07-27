#ifndef GRAPH_LAYOUTER_SETTINGS_H
#define GRAPH_LAYOUTER_SETTINGS_H

struct graph_layouter_settings
{
    bool conform_to_grid = true;

    // GATE TYPE
    // MODULE TYPE
    // NET TYPE

    bool separate_gnd = true;
    bool separate_vcc = true;
};

#endif // GRAPH_LAYOUTER_SETTINGS_H
