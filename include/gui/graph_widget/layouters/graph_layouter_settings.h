#ifndef GRAPH_LAYOUTER_SETTINGS_H
#define GRAPH_LAYOUTER_SETTINGS_H

class graph_layouter_settings
{
    // UNCERTAIN HOW TO IMPLEMENT LAYOUTER SETTINGS, DONT USE THIS CLASS YET
public:
    graph_layouter_settings();

    bool conform_to_grid() const;

protected:
    bool m_conform_to_grid;
    // GATE TYPE

    bool m_separate_gnd;
    bool m_separate_vcc;
};

#endif // GRAPH_LAYOUTER_SETTINGS_H
