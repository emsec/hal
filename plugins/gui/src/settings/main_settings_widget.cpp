#include "gui/settings/main_settings_widget.h"

#include "gui/expanding_list/expanding_list_button.h"
#include "gui/expanding_list/expanding_list_widget.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/sort.h"
#include "gui/searchbar/searchbar.h"
#include "gui/settings/checkbox_setting.h"
#include "gui/settings/dropdown_setting.h"
#include "gui/settings/fontsize_preview_widget.h"
#include "gui/settings/keybind_setting.h"
#include "gui/settings/settings_display.h"
#include "gui/settings/settings_widget.h"
#include "gui/settings/slider_setting.h"
#include "gui/settings/spinbox_setting.h"
#include "gui/settings/text_setting.h"

#include <QDebug>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>

// disable this (and enable SETTINGS_UPDATE_IMMEDIATELY in SettingsWidget.cpp)
// to apply all settings as they are modified
#define ENABLE_OK_BUTTON
namespace hal
{
    MainSettingsWidget::MainSettingsWidget(QWidget* parent)
        : QWidget(parent), mLayout(new QHBoxLayout()), mExpandingListWidget(new ExpandingListWidget()), mVerticalLayout(new QVBoxLayout()), mScrollbar(new QScrollBar()),
          mSearchbarContainer(new QFrame()), mSearchbarLayout(new QHBoxLayout()), mSearchbar(new Searchbar()), mScrollArea(new QScrollArea()), mContent(new QFrame()),
          mContentLayout(new QHBoxLayout()), mSettingsContainer(new QFrame()), mContainerLayout(new QVBoxLayout()), mButtonLayout(new QHBoxLayout()), mRestoreDefaults(new QPushButton()),
          mCancel(new QPushButton()), mOk(new QPushButton())
    {
        setWindowModality(Qt::ApplicationModal);

        setLayout(mLayout);
        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);
        mLayout->addWidget(mExpandingListWidget);
        mLayout->addLayout(mVerticalLayout);
        //mLayout->addWidget(mScrollbar);

        mVerticalLayout->setContentsMargins(0, 0, 0, 0);
        mVerticalLayout->setSpacing(0);
        mVerticalLayout->addWidget(mSearchbarContainer);

        mSearchbarContainer->setObjectName("Searchbar-container");
        mSearchbarContainer->setLayout(mSearchbarLayout);
        mSearchbarLayout->setContentsMargins(0, 0, 0, 0);
        mSearchbarLayout->setSpacing(0);
        mSearchbarLayout->addWidget(mSearchbar);
        mSearchbar->setModeButtonText("Settings");

        mScrollArea->setFrameStyle(QFrame::NoFrame);
        mVerticalLayout->addWidget(mScrollArea);

        mContent->setFrameStyle(QFrame::NoFrame);

        mContentLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        mContentLayout->setContentsMargins(0, 0, 0, 0);
        mContentLayout->setSpacing(0);
        mContent->setLayout(mContentLayout);

        mScrollArea->setWidget(mContent);
        mScrollArea->setWidgetResizable(true);

        mSettingsContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mSettingsContainer->setFrameStyle(QFrame::NoFrame);

        mContainerLayout->setAlignment(Qt::AlignLeft);
        mContainerLayout->setContentsMargins(0, 0, 0, 0);
        mContainerLayout->setSpacing(0);
        mSettingsContainer->setLayout(mContainerLayout);

        mContentLayout->addWidget(mSettingsContainer);

        mButtonLayout->setContentsMargins(10, 10, 10, 10);
        mButtonLayout->setSpacing(20);
        mButtonLayout->setAlignment(Qt::AlignRight);

        mVerticalLayout->addLayout(mButtonLayout);

        mRestoreDefaults->setText("Restore Defaults");
        mRestoreDefaults->setToolTip("Clear user preferences for this page");
        mCancel->setText("Cancel");
        mOk->setText("OK");
        mRestoreDefaults->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mCancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mOk->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        QWidget* spacer = new QWidget();
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        mButtonLayout->addWidget(mRestoreDefaults, Qt::AlignLeft);
        mButtonLayout->addWidget(spacer);
        mButtonLayout->addWidget(mCancel, Qt::AlignRight);
        mButtonLayout->addWidget(mOk, Qt::AlignRight);

        connect(mRestoreDefaults, &QPushButton::clicked, this, &MainSettingsWidget::handleRestoreDefaultsClicked);
        connect(mCancel, &QPushButton::clicked, this, &MainSettingsWidget::handleCancelClicked);
        connect(mOk, &QPushButton::clicked, this, &MainSettingsWidget::handleOkClicked);

        connect(mExpandingListWidget, &ExpandingListWidget::buttonSelected, this, &MainSettingsWidget::handleButtonSelected);
        connect(mSearchbar, &Searchbar::textEdited, this, &MainSettingsWidget::handleTextEdited);

    #ifndef ENABLE_OK_BUTTON
        mOk->hide();
    #endif

        initWidgets();

        mExpandingListWidget->selectItem(0);
        mExpandingListWidget->repolish();
    }

    void MainSettingsWidget::initWidgets()
    {
        makeExclusiveGroup("keybinds");
        makeExclusiveGroup("kbdmodifiers");

        QMap<QString, QVariant> standard_modifiers;
        standard_modifiers.insert("Shift", QVariant(Qt::ShiftModifier));
        standard_modifiers.insert("Ctrl", QVariant(Qt::ControlModifier));
        standard_modifiers.insert("Alt", QVariant(Qt::AltModifier));

        QMap<QString, QVariant> standard_sort_mechanisms;
        standard_sort_mechanisms.insert("Natural", QVariant(gui_utility::mSortMechanism::natural));
        standard_sort_mechanisms.insert("Lexical", QVariant(gui_utility::mSortMechanism::lexical));

        /************* ADD NEW SETTINGS WIDGETS HERE *************/

        makeSection("Style", "style-item", ":/icons/eye");

        QMap<QString, QVariant> theme_options;
        theme_options.insert("Darcula", "darcula");
        theme_options.insert("Sunny", "sunny");
        DropdownSetting* theme_settings = new DropdownSetting("main_style/theme", "Main Style Theme", theme_options, "will be set as your theme after restarting", this);
        //theme_settings->resetLabels();
        registerWidget("style-item", theme_settings);

        makeSection("Graph View", "graphview-item", ":/icons/graph");

        QMap<QString, QVariant> graph_grid_options;
        graph_grid_options.insert("None", "none");
        graph_grid_options.insert("Lines", "lines");
        graph_grid_options.insert("Dots", "dots");
        DropdownSetting* graph_grid_settings = new DropdownSetting("graph_view/grid_type", "Grid", graph_grid_options, "", this);
        registerWidget("graphview-item", graph_grid_settings);

        DropdownSetting* graph_dragswap_settings = new DropdownSetting("graph_view/drag_mode_modifier", "Move/Swap modifier", standard_modifiers, "toggles drag-and-drop mode", this);
        registerWidget("graphview-item", graph_dragswap_settings);
        assignExclusiveGroup("kbdmodifiers", graph_dragswap_settings);

        DropdownSetting* graph_movescene_settings = new DropdownSetting("graph_view/move_modifier", "Pan scene modifier", standard_modifiers, "lets you pan the scene", this);
        registerWidget("graphview-item", graph_movescene_settings);
        assignExclusiveGroup("kbdmodifiers", graph_movescene_settings);

        CheckboxSetting* graph_layout_parse = new CheckboxSetting("graph_view/layout_parse", "Apply parsed position", "enabled", "<--- use parsed verilog coordinates if any", this);
        registerWidget("graphview-item", graph_layout_parse);

        CheckboxSetting* graph_layout_boxes = new CheckboxSetting("graph_view/layout_boxes", "Optimize box layout", "enabled", "<--- fast random placement if disabled", this);
        registerWidget("graphview-item", graph_layout_boxes);

        CheckboxSetting* graph_layout_nets = new CheckboxSetting("graph_view/layout_nets", "Optimize net layout", "enabled", "<--- net optimization not fully tested", this);
        registerWidget("graphview-item", graph_layout_nets);

        makeSection("Navigation", "navigation-item", ":/icons/graph");

        DropdownSetting* nav_sort_mechanism_settings = new DropdownSetting("navigation/mSortMechanism", "Sort Mechanism for the Details View", standard_sort_mechanisms, "", this);
        registerWidget("navigation-item", nav_sort_mechanism_settings);

        makeSection("Python editor", "python-item", ":/icons/python");

        SliderSetting* py_font_size_setting          = new SliderSetting("python/sFontSize", "Font Size", 6, 40, "pt", this);
        FontsizePreviewWidget* py_font_size_preview = new FontsizePreviewWidget("foobar", font());
        py_font_size_preview->setMinimumSize(QSize(220, 85));
        py_font_size_setting->setPreviewWidget(py_font_size_preview);
        py_font_size_setting->setPreviewPosition(SettingsWidget::preview_position::right);
        registerWidget("python-item", py_font_size_setting);
        CheckboxSetting* py_line_numbers_setting = new CheckboxSetting("python/line_numbers", "Line Numbers", "show", "", this);
        registerWidget("python-item", py_line_numbers_setting);
        CheckboxSetting* py_line_highlight = new CheckboxSetting("python/highlightCurrentLine", "Highlight Current Line", "enabled", "", this);
        registerWidget("python-item", py_line_highlight);
        CheckboxSetting* py_line_wrap = new CheckboxSetting("python/line_wrap", "Line Wrap Mode", "wrap", "<-- prevents horizontal scrollbar", this);
        registerWidget("python-item", py_line_wrap);
        CheckboxSetting* py_minimap = new CheckboxSetting("python/minimap", "Minimap", "show", "", this);
        registerWidget("python-item", py_minimap);

        makeSection("Details window", "selection-details-window-item", ":/icons/gen-window");

        CheckboxSetting* selection_details_empty_sections_setting = new CheckboxSetting("selection_details/hide_empty_sections", "Hide empty sections", "enabled", "", this);
        registerWidget("selection-details-window-item", selection_details_empty_sections_setting);
        CheckboxSetting* selection_details_big_icon_setting = new CheckboxSetting("selection_details/show_big_icon", "Show big icon", "enabled", "", this);
        registerWidget("selection-details-window-item", selection_details_big_icon_setting);

        makeSection("Expert settings", "advanced-item", ":/icons/preferences");

        CheckboxSetting* autosave_setting = new CheckboxSetting("advanced/autosave", "Auto-save your project", "enabled", "", this);
        registerWidget("advanced-item", autosave_setting);
        SpinboxSetting* autosave_interval_setting = new SpinboxSetting("advanced/autosave_interval", "Auto-save interval", 30, 600, "s", this);
        registerWidget("advanced-item", autosave_interval_setting);

        makeSection("Keyboard Shortcuts", "keybind-item", ":/icons/keyboard");

        KeybindSetting* py_open_keybind = new KeybindSetting("keybinds/python_open_file", "Python: Open file", "", this);
        registerWidget("keybind-item", py_open_keybind);
        assignExclusiveGroup("keybinds", py_open_keybind);

        KeybindSetting* py_save_keybind = new KeybindSetting("keybinds/python_save_file", "Python: Save file", "", this);
        registerWidget("keybind-item", py_save_keybind);
        assignExclusiveGroup("keybinds", py_save_keybind);

        KeybindSetting* py_saveas_keybind = new KeybindSetting("keybinds/python_save_file_as", "Python: Save file as", "", this);
        registerWidget("keybind-item", py_saveas_keybind);
        assignExclusiveGroup("keybinds", py_saveas_keybind);

        KeybindSetting* py_run_keybind = new KeybindSetting("keybinds/python_run_file", "Python: Run current file", "", this);
        registerWidget("keybind-item", py_run_keybind);
        assignExclusiveGroup("keybinds", py_run_keybind);

        KeybindSetting* py_create_keybind = new KeybindSetting("keybinds/python_create_file", "Python: Create file", "", this);
        registerWidget("keybind-item", py_create_keybind);
        assignExclusiveGroup("keybinds", py_create_keybind);

        KeybindSetting* project_create_keybind = new KeybindSetting("keybinds/project_create_file", "Project: New empty netlist", "", this);
        registerWidget("keybind-item", project_create_keybind);
        assignExclusiveGroup("keybinds", project_create_keybind);

        KeybindSetting* project_open_keybind = new KeybindSetting("keybinds/project_open_file", "Project: Open HDL or project file", "", this);
        registerWidget("keybind-item", project_open_keybind);
        assignExclusiveGroup("keybinds", project_open_keybind);

        KeybindSetting* project_save_keybind = new KeybindSetting("keybinds/project_save_file", "Project: Save", "", this);
        registerWidget("keybind-item", project_save_keybind);
        assignExclusiveGroup("keybinds", project_save_keybind);

        KeybindSetting* schedule_run_keybind = new KeybindSetting("keybinds/schedule_run", "Schedule: Run scheduled plugins", "", this);
        registerWidget("keybind-item", schedule_run_keybind);
        assignExclusiveGroup("keybinds", schedule_run_keybind);

        KeybindSetting* graph_view_zoom_in_keybind = new KeybindSetting("keybinds/graph_view_zoom_in", "Graphview: Zoom in", "", this);
        registerWidget("keybind-item", graph_view_zoom_in_keybind);
        assignExclusiveGroup("keybinds", graph_view_zoom_in_keybind);

        KeybindSetting* graph_view_zoom_out_keybind = new KeybindSetting("keybinds/graph_view_zoom_out", "Graphview: Zoom out", "", this);
        registerWidget("keybind-item", graph_view_zoom_out_keybind);
        assignExclusiveGroup("keybinds", graph_view_zoom_out_keybind);

        // this keybind is used in several locations, depending on which widget has focus
        KeybindSetting* everywhere_search_keybind = new KeybindSetting("keybinds/searchbar_toggle", "Search", "opens/closes the search bar", this);
        registerWidget("keybind-item", everywhere_search_keybind);
        assignExclusiveGroup("keybinds", everywhere_search_keybind);

        makeSection("Debug", "debug-item", ":/icons/bug");

        #ifdef GUI_DEBUG_GRID
        CheckboxSetting* grid_debug_setting = new CheckboxSetting("debug/grid", "Show grid debug Overlay", "enabled", "<-- shows you how the layouter sees the graph", this);
        registerWidget("debug-item", grid_debug_setting);
        #endif

        // TextSetting* py_interpreter_setting = new TextSetting("python/interpreter", "Python Interpreter", "will be used after restart", "/path/to/python");
        // registerWidget("advanced-item", py_interpreter_setting);
    }

    void MainSettingsWidget::makeSection(const QString& label, const QString& name, const QString& iconPath)
    {
        ExpandingListButton* btn = new ExpandingListButton();
        btn->setObjectName(name);
        btn->setText(label);
        btn->setIconPath(iconPath);
        mExpandingListWidget->appendItem(btn);
        mSections.insert(name, btn);
    }

    bool MainSettingsWidget::checkConflict(SettingsWidget* widget, const QVariant& value) const
    {
        QString group_name = mExclusiveW2g.value(widget);
        // if widget is not bound to an exclusive group, accept any value
        if (group_name.isNull())
            return false;
        // we must accept >1 widgets with value "invalid"
        bool valid = value.isValid();
        // iterate over each member of the exclusive group and check that none
        // have the same value
        QList<SettingsWidget*>* widgets_in_group = mExclusiveG2w.value(group_name);
        bool conflict                             = false;
        for (SettingsWidget* w : *widgets_in_group)
        {
            if (w != widget)
            {
                bool thisConflict = valid && w->value() == value;
                conflict |= thisConflict;
                w->setConflicts(thisConflict);
                if (thisConflict)
                    qDebug() << "conflict between" << widget->key() << "and" << w->key();
            }
        }
        widget->setConflicts(conflict);
        return conflict;
    }

    void MainSettingsWidget::makeExclusiveGroup(const QString& name)
    {
        mExclusiveGroups.append(name);
        mExclusiveG2w.insert(name, new QList<SettingsWidget*>());
    }

    void MainSettingsWidget::releaseExclusiveGroup(const QString& group_name, SettingsWidget* widget)
    {
        mExclusiveW2g.remove(widget);
        QList<SettingsWidget*>* widgets_in_group = mExclusiveG2w.value(group_name);
        widgets_in_group->removeOne(widget);
    }

    void MainSettingsWidget::assignExclusiveGroup(const QString& group_name, SettingsWidget* widget)
    {
        // a widget must not be in more than 1 exclusive group
        assert(!mExclusiveW2g.contains(widget));
        // the group must exist
        assert(mExclusiveGroups.contains(group_name));

        mExclusiveW2g.insert(widget, group_name);
        QList<SettingsWidget*>* widgets_in_group = mExclusiveG2w.value(group_name);
        widgets_in_group->append(widget);
    }

    void MainSettingsWidget::registerWidget(const QString& section_name, SettingsWidget* widget)
    {
        QList<SettingsWidget*>* section_widgets;
        if (!(section_widgets = mMap.value(section_name)))
        {
            // lazy-init the sub list upon first use
            section_widgets = new QList<SettingsWidget*>();
            mMap.insert(section_name, section_widgets);
        }
        section_widgets->append(widget);
        mAllSettings.append(widget);
        mContainerLayout->addWidget(widget);
        connect(widget, &SettingsWidget::settingUpdated, this, &MainSettingsWidget::handleSettingUpdated);
    }

    bool MainSettingsWidget::handleAboutToClose()
    {
        bool dirty = false;
        for (SettingsWidget* widget : mAllSettings)
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
                return saveSettings();
            if (input == QMessageBox::No)
                rollbackSettings();
        }
        return true;
    }

    void MainSettingsWidget::handleRestoreDefaultsClicked()
    {
        QList<SettingsWidget*>* widget_list = mMap.value(mActiveSection, nullptr);

        if (!widget_list)
        {
            //LOG ERROR
            return;
        }

        for (SettingsWidget* widget : *widget_list)
        {
            // clear the setting and sync the widget to the default value of its
            // connected setting
            QString key          = widget->key();
            QVariant default_val = gSettingsManager->reset(key);
            widget->prepare(default_val, default_val);
            checkConflict(widget, widget->value());
        }
    }

    void MainSettingsWidget::handleCancelClicked()
    {
        rollbackSettings();
        Q_EMIT close();
    }

    void MainSettingsWidget::handleOkClicked()
    {
    #ifdef ENABLE_OK_BUTTON
        saveSettings();
    #endif
        Q_EMIT close();
    }

    void MainSettingsWidget::handleButtonSelected(ExpandingListButton* button)
    {
        //if a settings category button gets selected while the settings are filtered the filter resets
        //a filter reset is accompanied by the first category beeing autopicked as default because there are no more filtered results and one category needs to be active (see 'handleTextEdited')
        //this method resets the filter, but we want the clicked button and not the default as active thus disabeling it before clearing the filter
        mResetToFirstElement = false;
        mSearchbar->clear();
        mResetToFirstElement = true;

        hideAllSettings();
        removeAllHighlights();

        if (!button)
        {
            //LOG ERROR
            return;
        }
        // TODO check performance of this reverse lookup
        QString section_name                 = mSections.key(button);
        mActiveSection                     = section_name;
        QList<SettingsWidget*>* widget_list = mMap.value(section_name, nullptr);

        if (!widget_list)
        {
            //LOG ERROR
            return;
        }

        for (SettingsWidget* widget : *widget_list)
        {
            if (widget)
            {
                // sync the widget to the current value of its connected setting
                QString key          = widget->key();
                QVariant val         = gSettingsManager->get(key);
                QVariant default_val = gSettingsManager->getDefault(key);
                widget->prepare(val, default_val);
                // then display
                widget->show();
            }
        }
    }

    void MainSettingsWidget::handleTextEdited(const QString& text)
    {
        QString simplified = text.simplified();

        if (simplified.isEmpty())
        {
            removeAllHighlights();

            //checks if we need to default select the first settings category
            //true if searchbar got cleared by hand by user
            //false if searchbar got cleared because a settings category has been clicked
            if(mResetToFirstElement)
                mExpandingListWidget->selectItem(0);

            return;
        }

        mExpandingListWidget->selectButton(nullptr);

        for (SettingsWidget* widget : mAllSettings)
        {
            if (widget->matchLabels(simplified))
                widget->show();
            else
                widget->hide();
        }
    }

    void MainSettingsWidget::handleSettingUpdated(SettingsWidget* sender, const QString& key, const QVariant& value)
    {
    #ifdef SETTINGS_UPDATE_IMMEDIATELY
        bool conflicts = checkConflict(sender, value);
        if (!conflicts)
        {
            gSettingsManager->update(key, value);
        }
    #else
        Q_UNUSED(key);
        checkConflict(sender, value);
    #endif
    }

    void MainSettingsWidget::hideAllSettings()
    {
        for (SettingsWidget* widget : mAllSettings)
            widget->hide();
    }

    void MainSettingsWidget::showAllSettings()
    {
        for (SettingsWidget* widget : mAllSettings)
            widget->show();
    }

    void MainSettingsWidget::removeAllHighlights()
    {
        for (SettingsWidget* widget : mAllSettings)
            widget->resetLabels();
    }

    bool MainSettingsWidget::saveSettings()
    {
        for (SettingsWidget* widget : mAllSettings)
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
        for (SettingsWidget* widget : mAllSettings)
        {
            if (widget->dirty())
            {
                QString key    = widget->key();
                QVariant value = widget->value();
                widget->markSaved();
                gSettingsManager->update(key, value);
            }
        }
        return true;
    }

    void MainSettingsWidget::rollbackSettings()
    {
        for (SettingsWidget* widget : mAllSettings)
        {
            widget->handleRollback();
        }
    }
}
