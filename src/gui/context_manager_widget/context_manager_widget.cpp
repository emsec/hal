#include "context_manager_widget/context_manager_widget.h"
#include <QVBoxLayout>
#include <QStringListModel>
#include <QStringList>
#include <QListWidgetItem>
#include <QSize>
#include <QDebug>

context_manager_widget::context_manager_widget(QWidget *parent) : content_widget("Context Manager", parent), m_list_widget(new QListWidget())
{
    QListWidgetItem* item = new QListWidgetItem("Context String 1");

    m_list_widget->addItem(item);
    m_list_widget->addItem("Context String 2");
    m_list_widget->addItem("Context String 3");
 
    m_content_layout->addWidget(m_list_widget);
}

void context_manager_widget::resizeEvent(QResizeEvent* event)
{
   m_list_widget->setFixedWidth(event->size().width());
}
