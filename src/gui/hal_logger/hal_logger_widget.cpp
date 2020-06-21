#include "hal_logger/hal_logger_widget.h"
#include "channel_manager/channel_model.h"
#include "channel_manager/channel_selector.h"
#include "hal_logger/hal_filter_item.h"
#include "hal_logger/hal_filter_tab_bar.h"
#include "hal_logger/hal_logger_marshall.h"
#include "toolbar/toolbar.h"
#include <QHeaderView>

namespace hal
{
    HalLoggerWidget::HalLoggerWidget(QWidget* parent) : ContentWidget("Log", parent)
    {
        m_plain_text_edit = new QPlainTextEdit(this);
        m_plain_text_edit->setReadOnly(true);
        m_plain_text_edit->setFrameStyle(QFrame::NoFrame);

        m_plain_text_edit_scrollbar = m_plain_text_edit->verticalScrollBar();
        scroll_to_bottom();
        m_user_interacted_with_scrollbar = false;

        m_tab_bar      = new HalFilterTabBar();
        m_log_marshall = new HalLoggerMarshall(m_plain_text_edit);
        m_content_layout->addWidget(m_plain_text_edit);

        m_plain_text_edit->setContextMenuPolicy(Qt::CustomContextMenu);

        m_selector = new ChannelSelector();

        connect(m_plain_text_edit, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(show_text_edit_context_menu(const QPoint&)));
        connect(m_tab_bar, SIGNAL(currentChanged(int)), this, SLOT(filter_item_clicked(int)));
        connect(m_selector, SIGNAL(currentIndexChanged(int)), this, SLOT(handle_current_channel_changed(int)));
        connect(m_plain_text_edit_scrollbar, &QScrollBar::actionTriggered, this, &HalLoggerWidget::handle_first_user_interaction);

        ChannelModel* model = ChannelModel::get_instance();
        connect(model, SIGNAL(updated(spdlog::level::level_enum, std::string, std::string)), this, SLOT(handle_channel_updated(spdlog::level::level_enum, std::string, std::string)));
    }

    HalLoggerWidget::~HalLoggerWidget()
    {
        //cant set the parent correct, so just delete them in the constructor
        delete m_tab_bar;
        delete m_selector;
    }

    void HalLoggerWidget::setup_toolbar(Toolbar* Toolbar)
    {
        //Toolbar->add_widget(m_selector);

        //selector will be deleted within the toolbars destructor
        ChannelSelector* selector = new ChannelSelector();
        connect(selector, SIGNAL(currentIndexChanged(int)), this, SLOT(handle_current_channel_changed(int)));
        Toolbar->addWidget(selector);
    }

    QPlainTextEdit* HalLoggerWidget::get_plain_text_edit()
    {
        return m_plain_text_edit;
    }

    HalFilterTabBar* HalLoggerWidget::get_tab_bar()
    {
        return m_tab_bar;
    }

    void HalLoggerWidget::show_text_edit_context_menu(const QPoint& point)
    {
        Q_UNUSED(point)
    }

    void HalLoggerWidget::filter_item_clicked(const int& index)
    {
        Q_UNUSED(index)
        //reload_log_content();
    }

    void HalLoggerWidget::reload_log_content()
    {
        //    hal_task_status_item *item = static_cast<hal_task_status_item*>(m_task_manager->get_tree_view()->currentIndex().internalPointer());
        //    HalFilterItem *filter = m_tab_bar->get_current_filter();

        //        m_plain_text_edit->clear();
        //        QWriteLocker item_locker(item->get_lock());
        //        boost::circular_buffer<hal_task_log_entry*> *buffer = item->get_buffer();
        //        boost::circular_buffer<hal_task_log_entry*>::iterator itBegin = buffer->begin();
        //        boost::circular_buffer<hal_task_log_entry*>::iterator itEnd = buffer->end();
        //        for(; itBegin != itEnd; ++itBegin)
        //        {
        //            hal_task_log_entry *entry = *itBegin;
        //            m_log_marshall->append_log(entry->m_msg_type, QString::fromStdString(entry->m_msg), filter);
        //        }
    }    // item_locker scope

    //void HalLoggerWidget::handle_current_channel_changed(hal_channel_item* item)
    //{
    //    HalFilterItem* filter = m_tab_bar->get_current_filter();

    //    m_plain_text_edit->clear();
    //    QWriteLocker item_locker(item->get_lock());
    //    boost::circular_buffer<hal_channel_entry*>* buffer           = item->get_buffer();
    //    boost::circular_buffer<hal_channel_entry*>::iterator itBegin = buffer->begin();
    //    boost::circular_buffer<hal_channel_entry*>::iterator itEnd   = buffer->end();
    //    for (; itBegin != itEnd; ++itBegin)
    //    {
    //        hal_channel_entry* entry = *itBegin;
    //        m_log_marshall->append_log(entry->m_msg_type, QString::fromStdString(entry->m_msg), filter);
    //    }
    //}    // item_locker scope

    void HalLoggerWidget::handle_current_channel_updated(spdlog::level::level_enum t, const QString& msg)
    {
        HalFilterItem* filter = m_tab_bar->get_current_filter();
        m_log_marshall->append_log(t, msg, filter);
    }

    void HalLoggerWidget::handle_channel_updated(spdlog::level::level_enum t, const std::string& logger_name, const std::string& msg)
    {
        if (m_current_channel == "")
        {
            handle_current_channel_changed(0);
        }
        if (logger_name != m_current_channel)
            return;

        HalFilterItem* filter = m_tab_bar->get_current_filter();
        m_log_marshall->append_log(t, QString::fromStdString(msg), filter);
    }

    void HalLoggerWidget::handle_current_channel_changed(int index)
    {
        ChannelModel* model = ChannelModel::get_instance();
        ChannelItem* item   = static_cast<ChannelItem*>((model->index(index, 0, QModelIndex())).internalPointer());

        m_current_channel = item->name().toStdString();

        HalFilterItem* filter = m_tab_bar->get_current_filter();

        m_plain_text_edit->clear();
        QWriteLocker item_locker(item->get_lock());
        const boost::circular_buffer<channel_entry*>* buffer           = item->get_buffer();
        boost::circular_buffer<channel_entry*>::const_iterator itBegin = buffer->begin();
        boost::circular_buffer<channel_entry*>::const_iterator itEnd   = buffer->end();
        for (; itBegin != itEnd; ++itBegin)
        {
            channel_entry* entry = *itBegin;
            m_log_marshall->append_log(entry->m_msg_type, QString::fromStdString(entry->m_msg), filter);
        }
    }

    void HalLoggerWidget::handle_first_user_interaction(int value)
    {
        Q_UNUSED(value);

        if (!m_user_interacted_with_scrollbar)
            m_user_interacted_with_scrollbar = true;
    }

    void HalLoggerWidget::resizeEvent(QResizeEvent* event)
    {
        Q_UNUSED(event)

        if (!m_user_interacted_with_scrollbar)
            scroll_to_bottom();
    }

    void HalLoggerWidget::scroll_to_bottom()
    {
        m_plain_text_edit_scrollbar->setValue(m_plain_text_edit_scrollbar->maximum());
    }
}
