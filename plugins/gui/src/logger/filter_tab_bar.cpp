#include "gui/logger/filter_tab_bar.h"
#include "gui/logger/filter_item.h"
#include "gui/logger/logger_widget.h"

namespace hal
{
    FilterTabBar::FilterTabBar() : mDialog(this)
    {
        mButton.setText("+");
        mButton.setAutoRaise(true);
        connect(&mButton, SIGNAL(clicked()), &mDialog, SLOT(exec()));

        setExpanding(false);
        //setTabsClosable(true);
        addTab("Unfiltered");
        mFilterItems.append(nullptr);
        addTab("New Filter");
        setTabEnabled(1, false);
        setTabButton(1, QTabBar::RightSide, &mButton);
    }

    FilterTabBar::~FilterTabBar()
    {
    }

    void FilterTabBar::addNewFilter(QString name, FilterItem* item)
    {
        insertTab(count() - 1, name);
        mFilterItems.append(item);
    }

    FilterItem* FilterTabBar::getCurrentFilter()
    {
        return mFilterItems.value(this->currentIndex());
    }
}
