#include "gui/main_window/color_selection.h"

#include <QPainter>
#include <QColor>
#include <QColorDialog>

namespace hal {

    ColorSelection::ColorSelection(const QString& col, const QString &labl, bool bullet, QWidget* parent)
        : QPushButton(" ", parent), mColorName(col), mLabel(labl), mBullet(bullet)
    {
        setMaximumSize(QSize(32,24));
        setAutoFillBackground(true);
        connect(this,&QPushButton::clicked,this,&ColorSelection::execColorDialog);
    }

    void ColorSelection::paintEvent(QPaintEvent* evt)
    {
        Q_UNUSED(evt);
        QPainter painter(this);

        painter.setPen(QPen(Qt::black,0));
        QRectF r = rect();
        if (mBullet)
        {
            painter.setBrush(QBrush(QColor(mColorName)));
            int delta = (r.width() - r.height()) / 2;
            if (delta > 0)
            {
                r.setLeft(r.left() + delta);
                r.setRight(r.right() - delta);
            }
            else if (delta < 0)
            {
                r.setTop(r.top() - delta);
                r.setBottom(r.bottom() + delta);
            }
            painter.drawEllipse(r);
        }
        else
        {
            painter.drawRect(r);
            painter.setPen(QPen(QColor(mColorName),5.));
            int yc = r.top() + r.height()/2;
            painter.drawLine(r.left(),yc,r.right(),yc);
        }
    }

    void ColorSelection::execColorDialog()
    {
        QColor currentColor  = QColor(mColorName);
        QColor selectedColor = QColorDialog::getColor(currentColor, this, "Select color for " + mLabel);
        if (selectedColor.isValid() && selectedColor != currentColor)
        {
            mColorName = selectedColor.name();
            Q_EMIT colorChanged(mColorName);
        }
    }

}
