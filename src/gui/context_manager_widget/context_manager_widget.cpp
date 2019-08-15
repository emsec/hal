#include "context_manager_widget/context_manager_widget.h"
#include <QVBoxLayout>
#include <QStringListModel>
#include <QStringList>
#include <QListWidgetItem>
#include <QSize>

context_manager_widget::context_manager_widget(QWidget *parent) : content_widget("Context Manager", parent), m_list_widget(new QListWidget())
{
    //m_list_widget->setSizeAdjustPolicy(QListWidget::AdjustToContents);
    QStringListModel* mod = new QStringListModel();
    mod->setStringList(QStringList() << "hey" << "hoy");
    m_list_view = new QListView();
    m_list_view->setModel(mod);

    QListWidgetItem* item = new QListWidgetItem("TEST");

    m_list_widget->addItem(item);
    m_list_widget->addItem("test_context_string");
    m_list_widget->addItem("test_context_2");
    //m_list_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //m_list_widget->setResizeMode(QListView::Adjust);

    //m_content_layout->addWidget(m_list_view);
    m_content_layout->addWidget(m_list_widget);



}
