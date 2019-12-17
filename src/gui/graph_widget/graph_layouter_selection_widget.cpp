#include "graph_widget/graph_layouter_selection_widget.h"

graph_layouter_selection_widget::graph_layouter_selection_widget(/*old_graph_widget* widget,*/ QWidget* parent) : QWidget(parent)
{
    //m_widget = widget;

    m_horizontal_layout = new QHBoxLayout();
    m_horizontal_layout->setContentsMargins(0, 0, 0, 0);
    m_horizontal_layout->setSpacing(0);
    setLayout(m_horizontal_layout);
    m_list = new QListWidget();
    m_list->setMaximumWidth(200);
    m_list->setFrameStyle(QFrame::NoFrame);
    m_horizontal_layout->addWidget(m_list);
    m_vertical_layout = new QVBoxLayout();
    m_vertical_layout->setContentsMargins(0, 0, 0, 0);
    m_vertical_layout->setSpacing(0);
    m_horizontal_layout->addLayout(m_vertical_layout);
    m_description = new QTextEdit();
    //m_description->setReadOnly(true);
    m_description->setTextInteractionFlags(Qt::NoTextInteraction);
    m_description->viewport()->setCursor(QCursor(Qt::ArrowCursor));
    m_description->setFrameStyle(QFrame::NoFrame);
    m_vertical_layout->addWidget(m_description);
    m_button_layout = new QHBoxLayout();
    m_button_layout->setContentsMargins(10, 10, 10, 10);
    m_button_layout->setSpacing(20);
    m_vertical_layout->addLayout(m_button_layout);
    m_button_layout->setAlignment(Qt::AlignRight);

    //m_list->addItems(graph_layouter_manager::get_instance().get_layouters());

    //m_list->setStyleSheet("* {background-color: rgb(31, 34, 35);}");
    for (int i = 0; i < m_list->count(); i++)
    {
        QListWidgetItem* item = m_list->item(i);
        item->setSizeHint(QSize(item->sizeHint().height(), 30));
    }

    //connect list to selection detail widget

    //select first entry if available
    //    if (m_widget->get_layouter() != "")
    //    {
    //        //select first entry if available select correct item
    //        m_cancel = new QPushButton("cancel");
    //        m_button_layout->addWidget(m_cancel);
    //        connect(m_cancel, &QPushButton::clicked, this, &graph_layouter_selection_widget::cancel_clicked);
    //    }

    m_ok = new QPushButton("OK");
    m_button_layout->addWidget(m_ok);
    connect(m_ok, &QPushButton::clicked, this, &graph_layouter_selection_widget::ok_clicked);
    connect(m_list, &QListWidget::currentTextChanged, this, &graph_layouter_selection_widget::current_selection_changed);
}

void graph_layouter_selection_widget::ok_clicked(bool checked)
{
    Q_UNUSED(checked)
    //m_widget->subscribe(m_list->currentItem()->text());
    //m_widget->show_view();
}

void graph_layouter_selection_widget::cancel_clicked(bool checked)
{
    Q_UNUSED(checked)
    //m_widget->show_view();
}

void graph_layouter_selection_widget::current_selection_changed(QString layouter)
{
    Q_UNUSED(layouter);
    QString text = "<H1>";
    //text += graph_layouter_manager::get_instance().get_name(layouter);
    text += "</H1>";
    //text += graph_layouter_manager::get_instance().get_description(layouter);
    m_description->setHtml(text);
    //m_description->setHtml(graph_layouter_manager::get_instance().get_description(layouter));
}
