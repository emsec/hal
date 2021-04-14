#include "gui/selection_details_widget/disputed_big_icon.h"
#include "gui/settings/settings_items/settings_item_checkbox.h"
#include "gui/gui_globals.h"

namespace hal
{
    SettingsItemCheckbox* DisputedBigIcon::sShowIconSetting =
            new SettingsItemCheckbox(
                "Big Icon",
                "selection_details/show_big_icon",
                true,
                "Appearance:Selection Details",
                "Specifies wheter an big icon representing the current selection is shown in the Selection Details Widget."
                );

    DisputedBigIcon::DisputedBigIcon(const QString &iconName, QWidget *parent)
        : QLabel(parent)
    {
        mIsVisible = sShowIconSetting->value().toBool();
        connect(sShowIconSetting,&SettingsItemCheckbox::boolChanged,this,&DisputedBigIcon::setVisibleStatus);

        if (!iconName.isEmpty())
            setPixmap(QPixmap(QString(":/icons/%1").arg(iconName),"PNG").scaled(64,64));
        setFixedSize(68,68);
        showOrHide();
    }

    void DisputedBigIcon::setVisibleStatus(bool vis)
    {
        if (mIsVisible==vis) return; // nothing to do
        mIsVisible = vis;
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
