#ifndef GRAPHICS_WIDGET_H
#define GRAPHICS_WIDGET_H

#include "graph_widget/graph_graphics_view.h"

#include <QFrame>

class QHBoxLayout;
class QSlider;

class graphics_widget : public QFrame
{
    Q_OBJECT

    // THIS WIDGET WILL LIKELY BE REMOVED

public:
    explicit graphics_widget(QWidget* parent = nullptr);

    graph_graphics_view* view() const;

private Q_SLOTS:
    void zoom_in(int level = 1);
    void zoom_out(int level = 1);
    void update_matrix();

private:
    QHBoxLayout* m_layout;
    graph_graphics_view* m_view;
    QSlider* m_zoom_slider;
};

#endif // GRAPHICS_WIDGET_H
