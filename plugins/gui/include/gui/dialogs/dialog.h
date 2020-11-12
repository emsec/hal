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
        Q_PROPERTY(int xOffset READ xOffset WRITE setXOffset)
        Q_PROPERTY(int yOffset READ yOffset WRITE setYOffset)

    public:
        explicit Dialog(QWidget* parent = nullptr);

        void fadeIn();

        int xOffset() const;
        int yOffset() const;

        void setXOffset(const int offset);
        void setYOffset(const int offset);

        //void add_accept_button(const QString& text);
        //void add_reject_button(const QString& text);

    Q_SIGNALS:
        void offsetChanged();

    protected:
        virtual void fadeInStarted();
        virtual void fadeInFinished();

    private:
        QGraphicsOpacityEffect* mEffect;

        QPropertyAnimation* mXAnimation;
        QPropertyAnimation* mYAnimation;
        QPropertyAnimation* mOpacityAnimation;

        QParallelAnimationGroup* mAnimationGroup;

        int mXOffset;
        int mYOffset;
    };
}
