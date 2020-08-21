#include "expanding_list/expanding_list_widget.h"

#include "expanding_list/expanding_list_button.h"
#include "expanding_list/expanding_list_item.h"

#include <QFrame>
#include <QStyle>
#include <QVBoxLayout>

namespace hal
{
    ExpandingListWidget::ExpandingListWidget(QWidget* parent)
        : QScrollArea(parent), m_content(new QFrame()), m_content_layout(new QVBoxLayout()), m_spacer(new QFrame()), m_selected_button(nullptr), m_extended_item(nullptr), m_offset(0)
    {
        setFrameStyle(QFrame::NoFrame);
        setWidget(m_content);
        setWidgetResizable(true);
        m_content->setObjectName("content");
        m_content->setFrameStyle(QFrame::NoFrame);
        m_content->setLayout(m_content_layout);
        m_content_layout->setAlignment(Qt::AlignTop);
        m_content_layout->setContentsMargins(0, 0, 0, 0);
        m_content_layout->setSpacing(0);
        m_spacer->setObjectName("spacer");
        m_spacer->setFrameStyle(QFrame::NoFrame);
        m_content_layout->addWidget(m_spacer);
    }

    void ExpandingListWidget::append_item(ExpandingListButton* button, ExpandingListButton* parent_button)
    {
        if (parent_button)
        {
            for (ExpandingListItem* item : m_items)
                if (parent_button == item->parent_button())
                    item->append_child_button(button);
        }
        else
        {
            ExpandingListItem* item = new ExpandingListItem(button);
            m_items.append(item);
            m_content_layout->addWidget(item);
        }
        connect(button, &ExpandingListButton::clicked, this, &ExpandingListWidget::handle_clicked);
    }

    void ExpandingListWidget::select_button(ExpandingListButton* button)
    {
        if (button == m_selected_button)
            return;

        if (!button)
        {
            if (m_selected_button)
            {
                m_selected_button->set_selected(false);
                m_selected_button = nullptr;
            }

            if (m_extended_item)
            {
                m_extended_item->set_expanded(false);
                m_extended_item->collapse();
                m_extended_item = nullptr;
            }
            return;
        }

        for (ExpandingListItem* item : m_items)
            if (item->contains(button))
            {
                if (m_selected_button)
                    m_selected_button->set_selected(false);

                m_selected_button = button;
                m_selected_button->set_selected(true);

                if (item != m_extended_item)
                {
                    if (m_extended_item)
                    {
                        m_extended_item->set_expanded(false);
                        m_extended_item->collapse();
                    }

                    m_extended_item = item;
                    m_extended_item->set_expanded(true);
                    m_extended_item->expand();
                }
            }

        Q_EMIT button_selected(button);
    }

    void ExpandingListWidget::select_item(int index)
    {
        if (index < 0 || index >= m_items.size())
            return;

        select_button(m_items.at(index)->parent_button());
    }

    void ExpandingListWidget::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        for (ExpandingListItem* item : m_items)
            item->repolish();
    }

    void ExpandingListWidget::handle_clicked()
    {
        QObject* obj                  = sender();
        ExpandingListButton* button = static_cast<ExpandingListButton*>(obj);
        select_button(button);
    }
}
