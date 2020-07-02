#include "logger/filter_tab_bar.h"
#include "logger/filter_item.h"
#include "logger/logger_widget.h"

namespace hal
{
    FilterTabBar::FilterTabBar() : m_dialog(this)
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

    FilterTabBar::~FilterTabBar()
    {
    }

    void FilterTabBar::addNewFilter(QString name, FilterItem* item)
    {
        insertTab(count() - 1, name);
        m_filter_items.append(item);
    }

    FilterItem* FilterTabBar::get_current_filter()
    {
        return m_filter_items.value(this->currentIndex());
    }
}
