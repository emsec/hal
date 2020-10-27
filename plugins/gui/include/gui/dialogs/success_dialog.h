#pragma once

#include "gui/dialogs/dialog.h"

#include <QPainterPath>

namespace hal
{
    class SuccessDialog : public Dialog
    {
        Q_OBJECT
        Q_PROPERTY(int arrow_offset READ arrow_offset WRITE set_arrow_offset)

    public:
        enum class direction
        {
            left,
            right
        };

        explicit SuccessDialog(QWidget* parent = nullptr);

        void fade_in_started() override;
        void fade_in_finished() override;

        void start();
        void stop();

        void set_direction(const direction direction);

        int arrow_offset() const;
        void set_arrow_offset(const int offset);

    protected:
        void paintEvent(QPaintEvent* event) override;
        QSize sizeHint() const override;

    private:
        QPropertyAnimation* m_arrow_animation;

        int m_arrow_offset;

        QPainterPath m_arrow_left;
        QPainterPath m_arrow_right;

        qreal m_bar_height;
        qreal m_arrow_width;
        qreal m_arrow_length;

        direction m_direction;
    };
}
