#pragma once

#include <QFrame>

class QGraphicsOpacityEffect;
class QParallelAnimationGroup;
class QPropertyAnimation;

namespace hal
{
    class Dialog : public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(int x_offset READ x_offset WRITE set_x_offset)
        Q_PROPERTY(int y_offset READ y_offset WRITE set_y_offset)

    public:
        explicit Dialog(QWidget* parent = nullptr);

        void fade_in();

        int x_offset() const;
        int y_offset() const;

        void set_x_offset(const int offset);
        void set_y_offset(const int offset);

    Q_SIGNALS:
        void offset_changed();


    protected:
        virtual void fade_in_started();
        virtual void fade_in_finished();

    private:
        QGraphicsOpacityEffect* m_effect;

        QPropertyAnimation* m_x_animation;
        QPropertyAnimation* m_y_animation;
        QPropertyAnimation* m_opacity_animation;

        QParallelAnimationGroup* m_animation_group;

        int m_x_offset;
        int m_y_offset;
    };
}
