#include "settings/main_settings_widget.h"

#include "expanding_list/expanding_list_button.h"
#include "expanding_list/expanding_list_widget.h"
#include "gui_globals.h"
#include "gui_utils/sort.h"
#include "searchbar/searchbar.h"
#include "settings/checkbox_setting.h"
#include "settings/dropdown_setting.h"
#include "settings/fontsize_preview_widget.h"
#include "settings/keybind_setting.h"
#include "settings/settings_display.h"
#include "settings/settings_widget.h"
#include "settings/slider_setting.h"
#include "settings/spinbox_setting.h"
#include "settings/text_setting.h"

#include <QDebug>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>

// disable this (and enable SETTINGS_UPDATE_IMMEDIATELY in settings_widget.cpp)
// to apply all settings as they are modified
#define ENABLE_OK_BUTTON

main_settings_widget::main_settings_widget(QWidget* parent)
    : QWidget(parent), m_layout(new QHBoxLayout()), m_expanding_list_widget(new expanding_list_widget()), m_vertical_layout(new QVBoxLayout()), m_scrollbar(new QScrollBar()),
      m_searchbar_container(new QFrame()), m_searchbar_layout(new QHBoxLayout()), m_searchbar(new searchbar()), m_scroll_area(new QScrollArea()), m_content(new QFrame()),
      m_content_layout(new QHBoxLayout()), m_settings_container(new QFrame()), m_container_layout(new QVBoxLayout()), m_button_layout(new QHBoxLayout()), m_restore_defaults(new QPushButton()),
      m_cancel(new QPushButton()), m_ok(new QPushButton())
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

    m_vertical_layout->addLayout(m_button_layout);

    m_restore_defaults->setText("Restore Defaults");
    m_restore_defaults->setToolTip("Clear user preferences for this page");
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

    connect(m_expanding_list_widget, &expanding_list_widget::button_selected, this, &main_settings_widget::handle_button_selected);
    connect(m_searchbar, &searchbar::text_edited, this, &main_settings_widget::handle_text_edited);

#ifndef ENABLE_OK_BUTTON
    m_ok->hide();
#endif

    init_widgets();

    m_expanding_list_widget->select_item(0);
    m_expanding_list_widget->repolish();
}

void main_settings_widget::init_widgets()
{
    make_exclusive_group("keybinds");
    make_exclusive_group("kbdmodifiers");

    QMap<QString, QVariant> standard_modifiers;
    standard_modifiers.insert("Shift", QVariant(Qt::ShiftModifier));
    standard_modifiers.insert("Ctrl", QVariant(Qt::ControlModifier));
    standard_modifiers.insert("Alt", QVariant(Qt::AltModifier));

    QMap<QString, QVariant> standard_sort_mechanisms;
    standard_sort_mechanisms.insert("Natural", QVariant(gui_utility::sort_mechanism::natural));
    standard_sort_mechanisms.insert("Lexical", QVariant(gui_utility::sort_mechanism::lexical));

    /************* ADD NEW SETTINGS WIDGETS HERE *************/

    make_section("Style", "style-item", ":/icons/eye");

    QMap<QString, QVariant> theme_options;
    theme_options.insert("Darcula", "darcula");
    theme_options.insert("Sunny", "sunny");
    dropdown_setting* theme_settings = new dropdown_setting("main_style/theme", "Main Style Theme", theme_options, "will be set as your theme after restarting", this);
    //theme_settings->reset_labels();
    register_widget("style-item", theme_settings);

    make_section("Graph View", "graphview-item", ":/icons/graph");

    QMap<QString, QVariant> graph_grid_options;
    graph_grid_options.insert("None", "none");
    graph_grid_options.insert("Lines", "lines");
    graph_grid_options.insert("Dots", "dots");
    dropdown_setting* graph_grid_settings = new dropdown_setting("graph_view/grid_type", "Grid", graph_grid_options, "", this);
    register_widget("graphview-item", graph_grid_settings);

    dropdown_setting* graph_dragswap_settings = new dropdown_setting("graph_view/drag_mode_modifier", "Move/Swap modifier", standard_modifiers, "toggles drag-and-drop mode", this);
    register_widget("graphview-item", graph_dragswap_settings);
    assign_exclusive_group("kbdmodifiers", graph_dragswap_settings);

    dropdown_setting* graph_movescene_settings = new dropdown_setting("graph_view/move_modifier", "Pan scene modifier", standard_modifiers, "lets you pan the scene", this);
    register_widget("graphview-item", graph_movescene_settings);
    assign_exclusive_group("kbdmodifiers", graph_movescene_settings);

    make_section("Navigation", "navigation-item", ":/icons/graph");

    dropdown_setting* nav_sort_mechanism_settings = new dropdown_setting("navigation/sort_mechanism", "Sort Mechanism for the Details View", standard_sort_mechanisms, "", this);
    register_widget("navigation-item", nav_sort_mechanism_settings);

    make_section("Python editor", "python-item", ":/icons/python");

    slider_setting* py_font_size_setting          = new slider_setting("python/font_size", "Font Size", 6, 40, "pt", this);
    fontsize_preview_widget* py_font_size_preview = new fontsize_preview_widget("foobar", font());
    py_font_size_preview->setMinimumSize(QSize(220, 85));
    py_font_size_setting->set_preview_widget(py_font_size_preview);
    py_font_size_setting->set_preview_position(settings_widget::preview_position::right);
    register_widget("python-item", py_font_size_setting);
    checkbox_setting* py_line_numbers_setting = new checkbox_setting("python/line_numbers", "Line Numbers", "show", "", this);
    register_widget("python-item", py_line_numbers_setting);
    checkbox_setting* py_line_highlight = new checkbox_setting("python/highlight_current_line", "Highlight Current Line", "enabled", "", this);
    register_widget("python-item", py_line_highlight);
    checkbox_setting* py_line_wrap = new checkbox_setting("python/line_wrap", "Line Wrap Mode", "wrap", "<-- prevents horizontal scrollbar", this);
    register_widget("python-item", py_line_wrap);
    checkbox_setting* py_minimap = new checkbox_setting("python/minimap", "Minimap", "show", "", this);
    register_widget("python-item", py_minimap);

    make_section("Details window", "selection-details-window-item", ":/icons/gen-window");

    checkbox_setting* selection_details_empty_sections_setting = new checkbox_setting("selection_details/hide_empty_sections", "Hide empty sections", "enabled", "", this);
    register_widget("selection-details-window-item", selection_details_empty_sections_setting);

    make_section("Expert settings", "advanced-item", ":/icons/preferences");

    checkbox_setting* autosave_setting = new checkbox_setting("advanced/autosave", "Auto-save your project", "enabled", "", this);
    register_widget("advanced-item", autosave_setting);
    spinbox_setting* autosave_interval_setting = new spinbox_setting("advanced/autosave_interval", "Auto-save interval", 30, 600, "s", this);
    register_widget("advanced-item", autosave_interval_setting);

    make_section("Keyboard Shortcuts", "keybind-item", ":/icons/keyboard");

    keybind_setting* py_open_keybind = new keybind_setting("keybinds/python_open_file", "Python: Open file", "", this);
    register_widget("keybind-item", py_open_keybind);
    assign_exclusive_group("keybinds", py_open_keybind);

    keybind_setting* py_save_keybind = new keybind_setting("keybinds/python_save_file", "Python: Save file", "", this);
    register_widget("keybind-item", py_save_keybind);
    assign_exclusive_group("keybinds", py_save_keybind);

    keybind_setting* py_saveas_keybind = new keybind_setting("keybinds/python_save_file_as", "Python: Save file as", "", this);
    register_widget("keybind-item", py_saveas_keybind);
    assign_exclusive_group("keybinds", py_saveas_keybind);

    keybind_setting* py_run_keybind = new keybind_setting("keybinds/python_run_file", "Python: Run current file", "", this);
    register_widget("keybind-item", py_run_keybind);
    assign_exclusive_group("keybinds", py_run_keybind);

    keybind_setting* py_create_keybind = new keybind_setting("keybinds/python_create_file", "Python: Create file", "", this);
    register_widget("keybind-item", py_create_keybind);
    assign_exclusive_group("keybinds", py_create_keybind);

    keybind_setting* project_create_keybind = new keybind_setting("keybinds/project_create_file", "Project: New empty netlist", "", this);
    register_widget("keybind-item", project_create_keybind);
    assign_exclusive_group("keybinds", project_create_keybind);

    keybind_setting* project_open_keybind = new keybind_setting("keybinds/project_open_file", "Project: Open HDL or project file", "", this);
    register_widget("keybind-item", project_open_keybind);
    assign_exclusive_group("keybinds", project_open_keybind);

    keybind_setting* project_save_keybind = new keybind_setting("keybinds/project_save_file", "Project: Save", "", this);
    register_widget("keybind-item", project_save_keybind);
    assign_exclusive_group("keybinds", project_save_keybind);

    keybind_setting* schedule_run_keybind = new keybind_setting("keybinds/schedule_run", "Schedule: Run scheduled plugins", "", this);
    register_widget("keybind-item", schedule_run_keybind);
    assign_exclusive_group("keybinds", schedule_run_keybind);

    keybind_setting* graph_view_zoom_in_keybind = new keybind_setting("keybinds/graph_view_zoom_in", "Graphview: Zoom in", "", this);
    register_widget("keybind-item", graph_view_zoom_in_keybind);
    assign_exclusive_group("keybinds", graph_view_zoom_in_keybind);

    keybind_setting* graph_view_zoom_out_keybind = new keybind_setting("keybinds/graph_view_zoom_out", "Graphview: Zoom out", "", this);
    register_widget("keybind-item", graph_view_zoom_out_keybind);
    assign_exclusive_group("keybinds", graph_view_zoom_out_keybind);

    // this keybind is used in several locations, depending on which widget has focus
    keybind_setting* everywhere_search_keybind = new keybind_setting("keybinds/searchbar_toggle", "Search", "opens/closes the search bar", this);
    register_widget("keybind-item", everywhere_search_keybind);
    assign_exclusive_group("keybinds", everywhere_search_keybind);

    make_section("Debug", "debug-item", ":/icons/bug");

    #ifdef GUI_DEBUG_GRID
    checkbox_setting* grid_debug_setting = new checkbox_setting("debug/grid", "Show grid debug overlay", "enabled", "<-- shows you how the layouter sees the graph", this);
    register_widget("debug-item", grid_debug_setting);
    #endif

    // text_setting* py_interpreter_setting = new text_setting("python/interpreter", "Python Interpreter", "will be used after restart", "/path/to/python");
    // register_widget("advanced-item", py_interpreter_setting);
}

void main_settings_widget::make_section(const QString& label, const QString& name, const QString& icon_path)
{
    expanding_list_button* btn = new expanding_list_button();
    btn->setObjectName(name);
    btn->set_text(label);
    btn->set_icon_path(icon_path);
    m_expanding_list_widget->append_item(btn);
    m_sections.insert(name, btn);
}

bool main_settings_widget::check_conflict(settings_widget* widget, const QVariant& value) const
{
    QString group_name = m_exclusive_w2g.value(widget);
    // if widget is not bound to an exclusive group, accept any value
    if (group_name.isNull())
        return false;
    // we must accept >1 widgets with value "invalid"
    bool valid = value.isValid();
    // iterate over each member of the exclusive group and check that none
    // have the same value
    QList<settings_widget*>* widgets_in_group = m_exclusive_g2w.value(group_name);
    bool conflict                             = false;
    for (settings_widget* w : *widgets_in_group)
    {
        if (w != widget)
        {
            bool thisConflict = valid && w->value() == value;
            conflict |= thisConflict;
            w->set_conflicts(thisConflict);
            if (thisConflict)
                qDebug() << "conflict between" << widget->key() << "and" << w->key();
        }
    }
    widget->set_conflicts(conflict);
    return conflict;
}

void main_settings_widget::make_exclusive_group(const QString& name)
{
    m_exclusive_groups.append(name);
    m_exclusive_g2w.insert(name, new QList<settings_widget*>());
}

void main_settings_widget::release_exclusive_group(const QString& group_name, settings_widget* widget)
{
    m_exclusive_w2g.remove(widget);
    QList<settings_widget*>* widgets_in_group = m_exclusive_g2w.value(group_name);
    widgets_in_group->removeOne(widget);
}

void main_settings_widget::assign_exclusive_group(const QString& group_name, settings_widget* widget)
{
    // a widget must not be in more than 1 exclusive group
    assert(!m_exclusive_w2g.contains(widget));
    // the group must exist
    assert(m_exclusive_groups.contains(group_name));

    m_exclusive_w2g.insert(widget, group_name);
    QList<settings_widget*>* widgets_in_group = m_exclusive_g2w.value(group_name);
    widgets_in_group->append(widget);
}

void main_settings_widget::register_widget(const QString& section_name, settings_widget* widget)
{
    QList<settings_widget*>* section_widgets;
    if (!(section_widgets = m_map.value(section_name)))
    {
        // lazy-init the sub list upon first use
        section_widgets = new QList<settings_widget*>();
        m_map.insert(section_name, section_widgets);
    }
    section_widgets->append(widget);
    m_all_settings.append(widget);
    m_container_layout->addWidget(widget);
    connect(widget, &settings_widget::setting_updated, this, &main_settings_widget::handle_setting_updated);
}

bool main_settings_widget::handle_about_to_close()
{
    bool dirty = false;
    for (settings_widget* widget : m_all_settings)
    {
        if (widget->dirty())
        {
            dirty = true;
            break;
        }
    }
    if (dirty)
    {
        QMessageBox::StandardButton input = QMessageBox::question(this, "Unsaved settings", "You have unsaved settings that would be discarded.\nSave before leaving the settings page?", QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        if (input == QMessageBox::Cancel)
            return false;
        if (input == QMessageBox::Yes)
            return save_settings();
        if (input == QMessageBox::No)
            rollback_settings();
    }
    return true;
}

void main_settings_widget::handle_restore_defaults_clicked()
{
    QList<settings_widget*>* widget_list = m_map.value(m_active_section, nullptr);

    if (!widget_list)
    {
        //LOG ERROR
        return;
    }

    for (settings_widget* widget : *widget_list)
    {
        // clear the setting and sync the widget to the default value of its
        // connected setting
        QString key          = widget->key();
        QVariant default_val = g_settings_manager.reset(key);
        widget->prepare(default_val, default_val);
        check_conflict(widget, widget->value());
    }
}

void main_settings_widget::handle_cancel_clicked()
{
    rollback_settings();
    Q_EMIT close();
}

void main_settings_widget::handle_ok_clicked()
{
#ifdef ENABLE_OK_BUTTON
    save_settings();
#endif
    Q_EMIT close();
}

void main_settings_widget::handle_button_selected(expanding_list_button* button)
{
    m_searchbar->clear();
    hide_all_settings();
    remove_all_highlights();

    if (!button)
    {
        //LOG ERROR
        return;
    }
    // TODO check performance of this reverse lookup
    QString section_name                 = m_sections.key(button);
    m_active_section                     = section_name;
    QList<settings_widget*>* widget_list = m_map.value(section_name, nullptr);

    if (!widget_list)
    {
        //LOG ERROR
        return;
    }

    for (settings_widget* widget : *widget_list)
    {
        if (widget)
        {
            // sync the widget to the current value of its connected setting
            QString key          = widget->key();
            QVariant val         = g_settings_manager.get(key);
            QVariant default_val = g_settings_manager.get_default(key);
            widget->prepare(val, default_val);
            // then display
            widget->show();
        }
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

void main_settings_widget::handle_setting_updated(settings_widget* sender, const QString& key, const QVariant& value)
{
#ifdef SETTINGS_UPDATE_IMMEDIATELY
    bool conflicts = check_conflict(sender, value);
    if (!conflicts)
    {
        g_settings_manager.update(key, value);
    }
#else
    Q_UNUSED(key);
    check_conflict(sender, value);
#endif
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

bool main_settings_widget::save_settings()
{
    for (settings_widget* widget : m_all_settings)
    {
        if (widget->conflicts())
        {
            QMessageBox msg;
            msg.setText("Please resolve all conflicts first");
            msg.setDetailedText("You have settings that collide with each other.\n"
                                "Settings can't be saved while conflicts exist.");
            msg.setWindowTitle("Settings Manager");
            msg.exec();
            return false;
        }
    }
    for (settings_widget* widget : m_all_settings)
    {
        if (widget->dirty())
        {
            QString key    = widget->key();
            QVariant value = widget->value();
            widget->mark_saved();
            g_settings_manager.update(key, value);
        }
    }
    return true;
}

void main_settings_widget::rollback_settings()
{
    for (settings_widget* widget : m_all_settings)
    {
        widget->handle_rollback();
    }
}
