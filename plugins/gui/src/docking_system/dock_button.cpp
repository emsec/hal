#include "gui/docking_system/dock_button.h"
#include "gui/content_widget/content_widget.h"
#include "hal_core/utilities/log.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"
#include <QPainter>

namespace hal
{
    DockButton::DockButton(ContentWidget* widget, button_orientation orientation, QObject* eventFilter, QWidget* parent) : QToolButton(parent), mWidget(widget), mOrientation(orientation)
    {
        setText(mWidget->name());
        setCheckable(true);
        setStyleSheet("QToolButton { font-family                : \"Iosevka\";padding: 0; margin: 0; }");
        setIcon(mWidget->icon());
        installEventFilter(eventFilter);

        mIconSize       = 14;
        mWidthPadding   = 16;
        mHeightPadding  = 0;
        mRelativeHeight = 18;
        adjustSize();

        connect(this, &DockButton::clicked, this, &DockButton::handleClicked);
    }

    void DockButton::paintEvent(QPaintEvent* event)
    {
        Q_UNUSED(event)
        auto font = property("font").value<QFont>();
        QPainter painter(this);
        painter.setFont(font);
        painter.setPen(Qt::white);
        if (underMouse())
        {
            painter.fillRect(0, 0, mWidth, mHeight, QColor("#666769"));
        }
        else if (isChecked())
        {
            painter.fillRect(0, 0, mWidth, mHeight, QColor("#808080"));
        }

        switch (mOrientation)
        {
            case button_orientation::horizontal:
                painter.translate(0, -1);
                break;
            case button_orientation::vertical_up:
                painter.translate(-1, mHeight);
                painter.rotate(270);
                break;
            case button_orientation::vertical_down:
                painter.translate(mWidth + 1, 0);
                painter.rotate(90);
                break;
        }
        icon().paint(&painter, 0, (mRelativeHeight / 2) - (mIconSize / 2), mIconSize, (mRelativeHeight / 2) - (mIconSize / 2) + mIconSize);
        painter.drawText(QRectF(QRect(mIconSize, 0, mRelativeWidth, mRelativeHeight)), Qt::AlignVCenter, text());
    }

    void DockButton::adjustSize()
    {
        auto font = property("font").value<QFont>();
        //, QFont::PreferAntialias);
        QFontMetrics fm(font);
        int textwidth    = fm.width(text());
        mRelativeWidth = mIconSize + textwidth + mWidthPadding;

        if (mOrientation == button_orientation::horizontal)
        {
            mWidth  = mRelativeWidth;
            mHeight = mRelativeHeight;
        }
        else
        {
            mWidth  = mRelativeHeight;
            mHeight = mRelativeWidth;
        }

        setFixedHeight(mHeight);
        setFixedWidth(mWidth);
    }

    int DockButton::relativeWidth()
    {
        return mRelativeWidth;
    }

    void DockButton::handleClicked(bool mChecked)
    {
        if (mChecked)
            mWidget->open();
        else
            mWidget->close();
    }

    ContentWidget* DockButton::widget()
    {
        return mWidget;
    }

    void DockButton::hide()
    {
        QWidget::hide();
        mHidden = true;
    }

    void DockButton::show()
    {
        QWidget::show();
        mHidden = false;
    }

    bool DockButton::hidden()
    {
        return mHidden;
    }

    bool DockButton::available()
    {
        return mAvailable;
    }

    void DockButton::setAvailable(bool available)
    {
        mAvailable = available;
    }

    void DockButton::setRelativeHeight(int height)
    {
        mRelativeHeight = height;
        adjustSize();
    }
}
