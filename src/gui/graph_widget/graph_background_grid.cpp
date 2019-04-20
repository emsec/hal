#include "graph_widget/graph_background_grid.h"
#include <QPainter>

graph_background_grid::graph_background_grid(QWidget* parent) : QWidget(parent)
{
    m_grid_size    = 10;
    m_cluster_size = 5;
    m_draw_grid    = false;
}

void graph_background_grid::paintEvent(QPaintEvent*)
{
    if (m_draw_grid)
    {
        QPainter painter(this);

        int width  = QWidget::width();
        int height = QWidget::height();

        QVarLengthArray<QLine, 256> lines;

        //horizontal lines
        for (int y = m_grid_size; y < height; y += m_grid_size)
            lines.append(QLine(0, y, width, y));
        //vertical lines
        for (int x = m_grid_size; x < width; x += m_grid_size)
            lines.append(QLine(x, 0, x, height));

        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setPen(QColor(Qt::darkGray).lighter(10));
        painter.drawLines(lines.data(), lines.size());
    }
}

void graph_background_grid::set_grid_size(int size)
{
    m_grid_size = size;
    repaint();
}

void graph_background_grid::set_cluster_size(int size)
{
    m_cluster_size = size;
    repaint();
}

bool graph_background_grid::grid_visible()
{
    return m_draw_grid;
}

void graph_background_grid::set_draw_grid_false()
{
    m_draw_grid = false;
    this->update();
}

void graph_background_grid::set_draw_grid_true()
{
    m_draw_grid = true;
    this->update();
}
