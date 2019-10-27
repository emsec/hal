#ifndef GRAPH_WIDGET_CONSTANTS_H
#define GRAPH_WIDGET_CONSTANTS_H

#include "QtGlobal"

namespace graph_widget_constants
{
static const int font_size = 12;
static const int grid_size = 14;
static const int cluster_size = 8;

static const qreal gate_min_lod = 0.2; // if current lod < than this use lowest level of detail
static const qreal gate_max_lod = 0.4; // if current lod > than this use highest level of detail

static const qreal net_min_lod = 0.1; // if current lod < than this use lowest level of detail
static const qreal net_max_lod = 0.4; // if current lod > than this use highest level of detail

static const qreal separated_net_min_lod = 0.1; // if current lod < than this use lowest level of detail
static const qreal separated_net_max_lod = 0.4; // if current lod > than this use highest level of detail

static const qreal global_net_min_lod = 0.1; // if current lod < than this use lowest level of detail
static const qreal global_net_max_lod = 0.4; // if current lod > than this use highest level of detail

static const qreal net_fade_in_lod = 0.1;
static const qreal net_fade_out_lod = 0.4;

static const qreal grid_fade_start_lod = 0.4;
static const qreal grid_fade_end_lod = 1.0;

static const int drag_swap_sensitivity_distance = 100;

enum class grid_type
{
    lines,
    dots
};

}

#endif // GRAPH_WIDGET_CONSTANTS_H
