#include "gui/input_dialog/pingroup_selector_dialog.h"
#include "gui/gui_globals.h"

namespace hal
{
    PingroupSelectorDialog::PingroupSelectorDialog(const QString windowTitle, const QString infoText, Module *m, bool showOnlyMultiPinGroups, QWidget *parent, Qt::WindowFlags f) : ComboboxDialog(parent, f)
    {
        setWindowTitle(windowTitle);
        setInfoText(infoText);

        if(!m) return;
        mMod = m;
        for(const auto pingroup : m->get_pin_groups())
        {
            if(showOnlyMultiPinGroups)
            {
                if(pingroup->size() > 1)
                {
                    mCombobox->addItem(QString::fromStdString(pingroup->get_name()));
                    mIdList.append(pingroup->get_id());
                }
            }
            else
            {
                mCombobox->addItem(QString::fromStdString(pingroup->get_name()));
                mIdList.append(pingroup->get_id());
            }
        }
    }

    int PingroupSelectorDialog::getSelectedGroupId() const
    {
        return mIdList.isEmpty() ? -1 : mIdList.at(mCombobox->currentIndex());
    }

}
