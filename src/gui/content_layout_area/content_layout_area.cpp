#include "content_layout_area/content_layout_area.h"

#include "docking_system/dock_bar.h"
#include "docking_system/splitter_anchor.h"
#include "docking_system/tab_widget.h"
#include "splitter/splitter.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
namespace hal{
content_layout_area::content_layout_area(QWidget* parent)
    : QWidget(parent), m_top_level_layout(new QVBoxLayout()), m_second_level_layout(new QHBoxLayout()), m_third_level_layout(new QVBoxLayout()), m_fourth_level_layout(new QHBoxLayout()),
      m_splitter_layout(new QVBoxLayout()), m_central_layout(new QHBoxLayout()), m_spacer_layout(new QHBoxLayout()),

      m_vertical_splitter(new splitter(Qt::Vertical, this)), m_horizontal_splitter(new splitter(Qt::Horizontal, this)), m_left_splitter(new splitter(Qt::Vertical, this)),
      m_right_splitter(new splitter(Qt::Vertical, this)), m_bottom_splitter(new splitter(Qt::Horizontal, this)),

      m_left_dock(new dock_bar(Qt::Vertical, button_orientation::vertical_up, this)), m_right_dock(new dock_bar(Qt::Vertical, button_orientation::vertical_down, this)),
      m_bottom_dock(new dock_bar(Qt::Horizontal, button_orientation::horizontal, this)),

      m_bottom_container(new QWidget(this)), m_left_spacer(new QFrame(this)), m_right_spacer(new QFrame(this)),

      m_left_anchor(new splitter_anchor(m_left_dock, m_left_splitter, this)), m_right_anchor(new splitter_anchor(m_right_dock, m_right_splitter, this)),
      m_bottom_anchor(new splitter_anchor(m_bottom_dock, m_bottom_splitter, this)),

      m_tab_widget(new tab_widget(this))
{
    connect(m_left_anchor, &splitter_anchor::content_changed, this, &content_layout_area::update_left_dock_bar);
    connect(m_right_anchor, &splitter_anchor::content_changed, this, &content_layout_area::update_right_dock_bar);
    connect(m_bottom_anchor, &splitter_anchor::content_changed, this, &content_layout_area::update_bottom_dock_bar);

    m_left_dock->setObjectName("left-dock-bar");
    m_right_dock->setObjectName("right-dock-bar");
    m_bottom_dock->setObjectName("bottom-dock-bar");

    m_bottom_container->setObjectName("bottom-container");
    m_left_spacer->setObjectName("left-spacer");
    m_right_spacer->setObjectName("right-spacer");

    m_left_dock->hide();
    m_right_dock->hide();
    m_bottom_container->hide();

    m_left_spacer->hide();
    m_right_spacer->hide();

    m_left_splitter->hide();
    m_right_splitter->hide();
    m_bottom_splitter->hide();

    m_left_splitter->setChildrenCollapsible(false);
    m_left_splitter->setMinimumWidth(240);
    m_left_splitter->setMinimumHeight(240);

    m_right_splitter->setChildrenCollapsible(false);
    m_right_splitter->setMinimumWidth(240);
    m_right_splitter->setMinimumHeight(240);

    m_bottom_splitter->setChildrenCollapsible(false);
    m_bottom_splitter->setMinimumWidth(240);
    m_bottom_splitter->setMinimumHeight(240);

    m_vertical_splitter->setChildrenCollapsible(false);
    m_horizontal_splitter->setChildrenCollapsible(false);

    setLayout(m_top_level_layout);

    m_top_level_layout->setContentsMargins(0, 0, 0, 0);
    m_top_level_layout->setSpacing(0);
    m_top_level_layout->addLayout(m_second_level_layout);
    m_bottom_container->setLayout(m_spacer_layout);
    m_top_level_layout->addWidget(m_bottom_container);
    m_top_level_layout->setAlignment(Qt::AlignTop);

    m_spacer_layout->setContentsMargins(0, 0, 0, 0);
    m_spacer_layout->setSpacing(0);
    m_spacer_layout->addWidget(m_left_spacer, Qt::AlignLeft);
    m_spacer_layout->addWidget(m_bottom_dock);
    m_spacer_layout->addWidget(m_right_spacer, Qt::AlignRight);

    m_second_level_layout->setContentsMargins(0, 0, 0, 0);
    m_second_level_layout->setSpacing(0);
    m_second_level_layout->addWidget(m_left_dock, Qt::AlignLeft);
    m_second_level_layout->addLayout(m_third_level_layout);
    m_second_level_layout->addWidget(m_right_dock, Qt::AlignRight);

    m_third_level_layout->setContentsMargins(0, 0, 0, 0);
    m_third_level_layout->setSpacing(0);
    m_third_level_layout->addLayout(m_fourth_level_layout);

    m_fourth_level_layout->setContentsMargins(0, 0, 0, 0);
    m_fourth_level_layout->setSpacing(0);
    m_fourth_level_layout->addWidget(m_vertical_splitter);

    m_vertical_splitter->addWidget(m_horizontal_splitter);
    m_vertical_splitter->addWidget(m_bottom_splitter);

    m_horizontal_splitter->addWidget(m_left_splitter);
    m_horizontal_splitter->addWidget(m_tab_widget);
    m_horizontal_splitter->addWidget(m_right_splitter);
}

void content_layout_area::add_content(content_widget* widget, int index, content_anchor anchor)
{
    switch (anchor)
    {
        case content_anchor::center:
            m_tab_widget->add(widget, index);
            break;
        case content_anchor::left:
            m_left_anchor->add(widget, index);
            break;
        case content_anchor::right:
            m_right_anchor->add(widget, index);
            break;
        case content_anchor::bottom:
            m_bottom_anchor->add(widget, index);
            break;
    }
}

void content_layout_area::init_splitter_size(const QSize& size)
{
    int height = size.height() - 240;
    if (height > 0)
        m_vertical_splitter->setSizes(QList<int>{height, 240});
    else
        m_vertical_splitter->setSizes(QList<int>{0, 240});

    int width = size.width() - 600;
    if (width > 0)
        m_horizontal_splitter->setSizes(QList<int>{300, width, 300});
    else
        m_horizontal_splitter->setSizes(QList<int>{240, 120, 240});
}

void content_layout_area::update_left_dock_bar()
{
    if (m_left_dock->count())
    {
        m_left_dock->show();
        m_left_spacer->show();
    }
    else
    {
        m_left_dock->hide();
        m_left_spacer->hide();
    }
}

void content_layout_area::update_right_dock_bar()
{
    if (m_right_dock->count())
    {
        m_right_dock->show();
        m_right_spacer->show();
    }
    else
    {
        m_right_dock->hide();
        m_right_spacer->hide();
    }
}

void content_layout_area::update_bottom_dock_bar()
{
    if (m_bottom_dock->count())
        m_bottom_container->show();
    else
        m_bottom_container->hide();
}
}
