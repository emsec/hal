#ifndef GRAPH_LAYOUT_PROGRESS_WIDGET_H
#define GRAPH_LAYOUT_PROGRESS_WIDGET_H

#include <QFrame>

class QParallelAnimationGroup;
class QPropertyAnimation;

class graph_layout_progress_widget : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(int arrow_offset READ arrow_offset WRITE set_arrow_offset)

public:
    enum class direction
    {
        left,
        right
    };

    explicit graph_layout_progress_widget(QWidget* parent = nullptr);

    void start();
    void stop();

    void set_direction(const direction direction);

    int arrow_offset() const;
    void set_arrow_offset(const int offset);

protected:
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;

private:
//    QPropertyAnimation* m_x_animation;
    QPropertyAnimation* m_opacity_animation;
    QPropertyAnimation* m_arrow_animation;

    QParallelAnimationGroup* m_animation_group;

    int m_arrow_offset;

    QPainterPath m_arrow_left;
    QPainterPath m_arrow_right;

    qreal m_bar_height;
    qreal m_arrow_width;
    qreal m_arrow_length;

    direction m_direction;
};

#endif // GRAPH_LAYOUT_PROGRESS_WIDGET_H
