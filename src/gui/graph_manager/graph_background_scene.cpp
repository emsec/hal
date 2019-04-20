#include "graph_manager/graph_background_scene.h"

graph_background_scene::graph_background_scene(QObject* parent) : QGraphicsScene(parent)
{
    m_draw_grid = false;
}

void graph_background_scene::update_max_width(qreal width)
{
    m_max_width = std::max(m_max_width, width);
}

void graph_background_scene::update_max_height(qreal height)
{
    m_max_height = std::max(m_max_height, height);
}

void graph_background_scene::drawBackground(QPainter* painter, const QRectF& rect)
{
    if (m_draw_grid)
    {
        const int gridSize = 14;

        const qreal left = int(rect.left()) - (int(rect.left()) % gridSize);
        const qreal top  = int(rect.top()) - (int(rect.top()) % gridSize);

        QVarLengthArray<QLineF, 100> lines;

        for (qreal x = left; x < rect.right(); x += gridSize)
            lines.append(QLineF(x, rect.top(), x, rect.bottom()));
        for (qreal y = top; y < rect.bottom(); y += gridSize)
            lines.append(QLineF(rect.left(), y, rect.right(), y));

        painter->setRenderHint(QPainter::Antialiasing, false);

        painter->setPen(QColor(Qt::darkGray).lighter(10));
        painter->drawLines(lines.data(), lines.size());
        painter->setPen(QColor(204, 120, 50, 1));
        //painter->drawRect(sceneRect());
    }
}

void graph_background_scene::set_draw_grid_false()
{
    m_draw_grid = false;
    this->update();
}

void graph_background_scene::set_draw_grid_true()
{
    m_draw_grid = true;
    this->update();
}

bool graph_background_scene::is_grid_visible()
{
    return m_draw_grid;
}
