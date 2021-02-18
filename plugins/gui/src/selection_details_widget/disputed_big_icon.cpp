#include "gui/selection_details_widget/disputed_big_icon.h"
#include "gui/gui_globals.h"

namespace hal
{
    DisputedBigIcon::DisputedBigIcon(const QString &iconName, QWidget *parent)
        : QLabel(parent), mIsVisible(true)
    {
        if (!iconName.isEmpty())
            setPixmap(QPixmap(QString(":/icons/%1").arg(iconName),"PNG").scaled(64,64));
        setFixedSize(68,68);
        showOrHide();
    }

    void DisputedBigIcon::showOrHide()
    {
        if (mIsVisible)
            show();
        else
            hide();
    }
}
