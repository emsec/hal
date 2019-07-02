#ifndef NET_INFORMATION_POPUP_H
#define NET_INFORMATION_POPUP_H

#include <QGraphicsObject>

class graphics_gate;

class QParallelAnimationGroup;
class QPropertyAnimation;

class gate_navigation_popup : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(int arrow_offset READ arrow_offset WRITE set_arrow_offset)

public:
    enum class type
    {
        left,
        right
    };

    explicit gate_navigation_popup(type t, QGraphicsItem* parent = nullptr);

    void start(const QPointF& final_position);
    void stop();

    qreal width() const;
    qreal height() const;

    void set_width(const qreal width);
    void set_height(const qreal height);

    int arrow_offset() const;
    void set_arrow_offset(const int offset);

    void set_graphics_gate(graphics_gate* g);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) Q_DECL_OVERRIDE;
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;
    virtual QPainterPath shape() const Q_DECL_OVERRIDE;

private:
    static bool s_delegate_paint;

    qreal m_width;
    qreal m_height;

    QPropertyAnimation* m_x_animation;
    QPropertyAnimation* m_opacity_animation;
    QPropertyAnimation* m_arrow_animation;

    QParallelAnimationGroup* m_animation_group;

    type m_type;

    int m_arrow_offset;

    graphics_gate* m_graphics_gate;
};

#endif // NET_INFORMATION_POPUP_H
