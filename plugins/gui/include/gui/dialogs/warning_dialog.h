#pragma once

#include "gui/dialogs/dialog.h"

#include <QPainterPath>

namespace hal
{
    class WarningDialog : public Dialog
    {
        Q_OBJECT
        Q_PROPERTY(qreal line_offset READ line_offset WRITE set_line_offset)

    public:
        enum class direction
        {
            left,
            right
        };

        explicit WarningDialog(QWidget* parent = nullptr, bool animate = true);

        void fade_in_started() override;
        void fade_in_finished() override;

        void set_direction(const direction direction);

        int line_offset() const;
        void set_line_offset(const int offset);

    protected:
        void paintEvent(QPaintEvent* event) override;
        QSize sizeHint() const override;

    private:
        QPropertyAnimation* m_line_animation;

        QPainterPath m_left_leaning_line;
        QPainterPath m_right_leaning_line;

        int m_line_offset;

        int m_bar_height;
        int m_line_width;
        int m_line_gradient;
        int m_line_spacing;

        bool m_animate;

        direction m_direction;
    };
}
