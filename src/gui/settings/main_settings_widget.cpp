#include "settings/main_settings_widget.h"

#include "expanding_list/expanding_list_button.h"
#include "expanding_list/expanding_list_widget.h"
#include "searchbar/searchbar.h"
#include "settings/bigger_example_settings.h"
#include "settings/example_settings.h"
#include "settings/settings_display.h"
#include "settings/settings_widget.h"
#include "settings/gui_theme_settings.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>

main_settings_widget::main_settings_widget(QWidget* parent)
    : QWidget(parent), m_layout(new QHBoxLayout()), m_expanding_list_widget(new expanding_list_widget()), m_vertical_layout(new QVBoxLayout()), m_scrollbar(new QScrollBar()),
      m_searchbar_container(new QFrame()), m_searchbar_layout(new QHBoxLayout()), m_searchbar(new searchbar()), m_settings_display(new settings_display()), m_scroll_area(new QScrollArea()),
      m_content(new QFrame()), m_content_layout(new QHBoxLayout()), m_settings_container(new QFrame()), m_container_layout(new QVBoxLayout()), m_button_layout(new QHBoxLayout()),
      m_restore_defaults(new QPushButton()), m_cancel(new QPushButton()), m_ok(new QPushButton())
{
    setWindowModality(Qt::ApplicationModal);

    setLayout(m_layout);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addWidget(m_expanding_list_widget);
    m_layout->addLayout(m_vertical_layout);
    //m_layout->addWidget(m_scrollbar);

    m_vertical_layout->setContentsMargins(0, 0, 0, 0);
    m_vertical_layout->setSpacing(0);
    m_vertical_layout->addWidget(m_searchbar_container);

    m_searchbar_container->setObjectName("searchbar-container");
    m_searchbar_container->setLayout(m_searchbar_layout);
    m_searchbar_layout->setContentsMargins(0, 0, 0, 0);
    m_searchbar_layout->setSpacing(0);
    m_searchbar_layout->addWidget(m_searchbar);
    m_searchbar->set_mode_button_text("Settings");

    m_settings_display->setFrameStyle(QFrame::NoFrame);
    //m_vertical_layout->addWidget(m_settings_display);
    m_scroll_area->setFrameStyle(QFrame::NoFrame);
    m_vertical_layout->addWidget(m_scroll_area);

    m_content->setFrameStyle(QFrame::NoFrame);

    m_content_layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_content_layout->setContentsMargins(0, 0, 0, 0);
    m_content_layout->setSpacing(0);
    m_content->setLayout(m_content_layout);

    m_scroll_area->setWidget(m_content);
    m_scroll_area->setWidgetResizable(true);

    m_settings_container->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_settings_container->setFrameStyle(QFrame::NoFrame);

    m_container_layout->setAlignment(Qt::AlignLeft);
    m_container_layout->setContentsMargins(0, 0, 0, 0);
    m_container_layout->setSpacing(0);
    m_settings_container->setLayout(m_container_layout);

    m_content_layout->addWidget(m_settings_container);

    m_button_layout->setContentsMargins(10, 10, 10, 10);
    m_button_layout->setSpacing(20);
    m_button_layout->setAlignment(Qt::AlignRight);

    //m_vertical_layout->addLayout(m_button_layout);

    m_restore_defaults->setText("Restore Defaults");
    m_cancel->setText("Cancel");
    m_ok->setText("OK");

    m_restore_defaults->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_button_layout->addWidget(m_restore_defaults, Qt::AlignLeft);
    m_button_layout->addWidget(spacer);
    m_button_layout->addWidget(m_cancel, Qt::AlignRight);
    m_button_layout->addWidget(m_ok, Qt::AlignRight);

    connect(m_restore_defaults, &QPushButton::clicked, this, &main_settings_widget::handle_restore_defaults_clicked);
    connect(m_cancel, &QPushButton::clicked, this, &main_settings_widget::handle_cancel_clicked);
    connect(m_ok, &QPushButton::clicked, this, &main_settings_widget::handle_ok_clicked);

    connect(m_expanding_list_widget, &expanding_list_widget::button_selected, this, &main_settings_widget::handel_button_selected);
    connect(m_searchbar, &searchbar::text_edited, this, &main_settings_widget::handle_text_edited);

    expanding_list_button* general_button       = new expanding_list_button();
    expanding_list_button* content_button       = new expanding_list_button();
    expanding_list_button* plugins_button       = new expanding_list_button();
    expanding_list_button* style_button         = new expanding_list_button();
    expanding_list_button* notifications_button = new expanding_list_button();
    expanding_list_button* debug_button         = new expanding_list_button();
    expanding_list_button* test_button1         = new expanding_list_button();
    expanding_list_button* test_button2         = new expanding_list_button();
    expanding_list_button* test_button3         = new expanding_list_button();
    expanding_list_button* test_button4         = new expanding_list_button();

    m_expanding_list_widget->append_item(general_button);
    m_expanding_list_widget->append_item(content_button);
    m_expanding_list_widget->append_item(plugins_button);
    m_expanding_list_widget->append_item(style_button);
    m_expanding_list_widget->append_item(notifications_button);
    m_expanding_list_widget->append_item(debug_button);
    m_expanding_list_widget->append_item(test_button1, debug_button);
    m_expanding_list_widget->append_item(test_button2, debug_button);
    m_expanding_list_widget->append_item(test_button3, debug_button);
    m_expanding_list_widget->append_item(test_button4, debug_button);

    general_button->set_icon_path(":/icons/settings");
    content_button->set_icon_path(":/icons/content");
    plugins_button->set_icon_path(":/icons/plugin");
    style_button->set_icon_path(":/icons/eye");
    notifications_button->set_icon_path(":/icons/bell");
    debug_button->set_icon_path(":/icons/fire");
    test_button1->set_icon_path(":/icons/search");
    test_button2->set_icon_path(":/icons/search");
    test_button3->set_icon_path(":/icons/search");
    test_button4->set_icon_path(":/icons/search");

    general_button->set_text("General");
    content_button->set_text("Content");
    plugins_button->set_text("Plugins");
    style_button->set_text("Style");
    notifications_button->set_text("Notifications");
    debug_button->set_text("Debug / Test");
    test_button1->set_text("Test 1");
    test_button2->set_text("Test 2");
    test_button3->set_text("Test 3");
    test_button4->set_text("Test 4");

    general_button->setObjectName("general-item");
    content_button->setObjectName("content-item");
    plugins_button->setObjectName("plugins-item");
    notifications_button->setObjectName("plugins-item");
    style_button->setObjectName("style-item");
    debug_button->setObjectName("debug-item");

    m_map.insert(general_button, &m_general_settings);
    m_map.insert(content_button, &m_content_settings);
    m_map.insert(plugins_button, &m_plugins_settings);
    m_map.insert(style_button, &m_style_settings);
    m_map.insert(notifications_button, &m_notifications_settings);
    m_map.insert(debug_button, &m_debug_settings);

    //demo to demonstrate settings
    gui_theme_settings* theme_settings = new gui_theme_settings();
    m_all_settings.append(theme_settings);
    m_style_settings.append(theme_settings);
    m_container_layout->addWidget(theme_settings);
    theme_settings->reset_labels();
    //end of demo

    example_settings* example = new example_settings();
    m_all_settings.append(example);
    m_debug_settings.append(example);
    m_container_layout->addWidget(example);
    example->reset_labels();

    bigger_example_settings* bigger_example = new bigger_example_settings();
    m_all_settings.append(bigger_example);
    m_debug_settings.append(bigger_example);
    m_container_layout->addWidget(bigger_example);
    bigger_example->reset_labels();

    m_expanding_list_widget->select_item(0);
    m_expanding_list_widget->repolish();
}

void main_settings_widget::handle_restore_defaults_clicked()
{
}

void main_settings_widget::handle_cancel_clicked()
{
    Q_EMIT close();
}

void main_settings_widget::handle_ok_clicked()
{
    Q_EMIT close();
}

void main_settings_widget::handel_button_selected(expanding_list_button* button)
{
    m_searchbar->clear();
    hide_all_settings();
    remove_all_highlights();

    if (!button)
    {
        //LOG ERROR
        return;
    }

    QMap<expanding_list_button*, QList<settings_widget*>*>::const_iterator iterator = m_map.find(button);

    if (iterator == m_map.constEnd())
    {
        //LOG ERROR
        return;
    }

    for (settings_widget* widget : *iterator.value())
    {
        if (widget)
            widget->show();
    }
}

void main_settings_widget::handle_text_edited(const QString& text)
{
    QString simplified = text.simplified();

    if (simplified.isEmpty())
    {
        remove_all_highlights();
        m_expanding_list_widget->select_item(0);
        return;
    }

    m_expanding_list_widget->select_button(nullptr);

    for (settings_widget* widget : m_all_settings)
    {
        if (widget->match_labels(simplified))
            widget->show();
        else
            widget->hide();
    }
}

void main_settings_widget::hide_all_settings()
{
    for (settings_widget* widget : m_all_settings)
        widget->hide();
}

void main_settings_widget::show_all_settings()
{
    for (settings_widget* widget : m_all_settings)
        widget->show();
}

void main_settings_widget::remove_all_highlights()
{
    for (settings_widget* widget : m_all_settings)
        widget->reset_labels();
}
