#pragma once

#include "gui/dialogs/dialog.h"

#include <QPainterPath>

namespace hal
{
    class SuccessDialog : public Dialog
    {
        Q_OBJECT
        Q_PROPERTY(int arrowOffset READ arrowOffset WRITE setArrowOffset)

    public:
        enum class direction
        {
            left,
            right
        };

        explicit SuccessDialog(QWidget* parent = nullptr);

        void fadeInStarted() override;
        void fadeInFinished() override;

        void start();
        void stop();

        void setDirection(const direction direction);

        int arrowOffset() const;
        void setArrowOffset(const int offset);

    protected:
        void paintEvent(QPaintEvent* event) override;
        QSize sizeHint() const override;

    private:
        QPropertyAnimation* mArrowAnimation;

        int mArrowOffset;

        QPainterPath mArrowLeft;
        QPainterPath mArrowRight;

        qreal mBarHeight;
        qreal mArrowWidth;
        qreal mArrowLength;

        direction mDirection;
    };
}
