#include "hal_logger/hal_filter_tab_bar.h"
#include "hal_logger/hal_filter_item.h"
#include "hal_logger/hal_logger_widget.h"

namespace hal
{
    HalFilterTabBar::HalFilterTabBar() : m_dialog(this)
    {
        m_button.setText("+");
        m_button.setAutoRaise(true);
        connect(&m_button, SIGNAL(clicked()), &m_dialog, SLOT(exec()));

        setExpanding(false);
        //setTabsClosable(true);
        addTab("Unfiltered");
        m_filter_items.append(nullptr);
        addTab("New Filter");
        setTabEnabled(1, false);
        setTabButton(1, QTabBar::RightSide, &m_button);
    }

    HalFilterTabBar::~HalFilterTabBar()
    {
    }

    void HalFilterTabBar::addNewFilter(QString name, HalFilterItem* item)
    {
        insertTab(count() - 1, name);
        m_filter_items.append(item);
    }

    HalFilterItem* HalFilterTabBar::get_current_filter()
    {
        return m_filter_items.value(this->currentIndex());
    }
}
