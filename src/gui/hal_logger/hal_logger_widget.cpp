#include "hal_logger/hal_logger_widget.h"
#include "channel_manager/channel_model.h"
#include "channel_manager/channel_selector.h"
#include "hal_logger/hal_filter_item.h"
#include "hal_logger/hal_filter_tab_bar.h"
#include "hal_logger/hal_logger_marshall.h"
#include "toolbar/toolbar.h"
#include <QHeaderView>

hal_logger_widget::hal_logger_widget(QWidget* parent) : content_widget("Log", parent)
{
    m_plain_text_edit = new QPlainTextEdit(this);
    m_plain_text_edit->setReadOnly(true);
    m_plain_text_edit->setFrameStyle(QFrame::NoFrame);

    m_plain_text_edit_scrollbar = m_plain_text_edit->verticalScrollBar();
    scroll_to_bottom();
    m_user_interacted_with_scrollbar = false;

    m_tab_bar      = new hal_filter_tab_bar();
    m_log_marshall = new hal_logger_marshall(m_plain_text_edit);
    m_content_layout->addWidget(m_plain_text_edit);

    m_plain_text_edit->setContextMenuPolicy(Qt::CustomContextMenu);

    m_selector = new channel_selector();

    connect(m_plain_text_edit, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(show_text_edit_context_menu(const QPoint&)));
    connect(m_tab_bar, SIGNAL(currentChanged(int)), this, SLOT(filter_item_clicked(int)));
    connect(m_selector, SIGNAL(currentIndexChanged(int)), this, SLOT(handle_current_channel_changed(int)));
    connect(m_plain_text_edit_scrollbar, &QScrollBar::actionTriggered, this, &hal_logger_widget::handle_first_user_interaction);

    channel_model* model = channel_model::get_instance();
    connect(model, SIGNAL(updated(spdlog::level::level_enum, std::string, std::string)), this, SLOT(handle_channel_updated(spdlog::level::level_enum, std::string, std::string)));
}

hal_logger_widget::~hal_logger_widget()
{
    //cant set the parent correct, so just delete them in the constructor
    delete m_tab_bar;
    delete m_selector;
}

void hal_logger_widget::setup_toolbar(toolbar* toolbar)
{
    //toolbar->add_widget(m_selector);

    //selector will be deleted within the toolbars destructor
    channel_selector* selector = new channel_selector();
    connect(selector, SIGNAL(currentIndexChanged(int)), this, SLOT(handle_current_channel_changed(int)));
    toolbar->addWidget(selector);
}

QPlainTextEdit* hal_logger_widget::get_plain_text_edit()
{
    return m_plain_text_edit;
}

hal_filter_tab_bar* hal_logger_widget::get_tab_bar()
{
    return m_tab_bar;
}

void hal_logger_widget::show_text_edit_context_menu(const QPoint& point)
{
    Q_UNUSED(point)
}

void hal_logger_widget::filter_item_clicked(const int& index)
{
    Q_UNUSED(index)
    //reload_log_content();
}

void hal_logger_widget::reload_log_content()
{
    //    hal_task_status_item *item = static_cast<hal_task_status_item*>(m_task_manager->get_tree_view()->currentIndex().internalPointer());
    //    hal_filter_item *filter = m_tab_bar->get_current_filter();

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

//void hal_logger_widget::handle_current_channel_changed(hal_channel_item* item)
//{
//    hal_filter_item* filter = m_tab_bar->get_current_filter();

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

void hal_logger_widget::handle_current_channel_updated(spdlog::level::level_enum t, const QString& msg)
{
    hal_filter_item* filter = m_tab_bar->get_current_filter();
    m_log_marshall->append_log(t, msg, filter);
}

void hal_logger_widget::handle_channel_updated(spdlog::level::level_enum t, const std::string& logger_name, const std::string& msg)
{
    if (m_current_channel == "")
    {
        handle_current_channel_changed(0);
    }
    if (logger_name != m_current_channel)
        return;

    hal_filter_item* filter = m_tab_bar->get_current_filter();
    m_log_marshall->append_log(t, QString::fromStdString(msg), filter);
}

void hal_logger_widget::handle_current_channel_changed(int index)
{
    channel_model* model = channel_model::get_instance();
    channel_item* item   = static_cast<channel_item*>((model->index(index, 0, QModelIndex())).internalPointer());

    m_current_channel = item->name().toStdString();

    hal_filter_item* filter = m_tab_bar->get_current_filter();

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

void hal_logger_widget::handle_first_user_interaction(int value)
{
    Q_UNUSED(value);

    if (!m_user_interacted_with_scrollbar)
        m_user_interacted_with_scrollbar = true;
}

void hal_logger_widget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)

    if (!m_user_interacted_with_scrollbar)
        scroll_to_bottom();
}

void hal_logger_widget::scroll_to_bottom()
{
    m_plain_text_edit_scrollbar->setValue(m_plain_text_edit_scrollbar->maximum());
}
