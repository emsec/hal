#pragma once

#include "gui/dialogs/dialog.h"

#include <QPainterPath>

namespace hal
{
    class WarningDialog : public Dialog
    {
        Q_OBJECT
        Q_PROPERTY(qreal lineOffset READ lineOffset WRITE setLineOffset)

    public:
        enum class direction
        {
            left,
            right
        };

        explicit WarningDialog(QWidget* parent = nullptr, bool animate = true);

        void fadeInStarted() override;
        void fadeInFinished() override;

        void setDirection(const direction direction);

        int lineOffset() const;
        void setLineOffset(const int offset);

    protected:
        void paintEvent(QPaintEvent* event) override;
        QSize sizeHint() const override;

    private:
        QPropertyAnimation* mLineAnimation;

        QPainterPath mLeftLeaningLine;
        QPainterPath mRightLeaningLine;

        int mLineOffset;

        int mBarHeight;
        int mLineWidth;
        int mLineGradient;
        int mLineSpacing;

        bool mAnimate;

        direction mDirection;
    };
}
