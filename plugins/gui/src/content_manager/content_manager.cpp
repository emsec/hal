#include "gui/content_manager/content_manager.h"
#include "gui/plugin_relay/gui_plugin_manager.h"
#include "gui/content_widget/content_widget.h"
#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/docking_system/tab_widget.h"
#include "gui/file_manager/file_manager.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/graph_widget/graph_context_manager.h"
#include "gui/graph_widget/graph_context_serializer.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_proxy_model.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/sort.h"
#include "gui/gui_utils/special_log_content_manager.h"
#include "gui/logger/logger_widget.h"
#include "gui/main_window/main_window.h"
#include "gui/module_model/module_proxy_model.h"
#include "gui/module_widget/module_widget.h"
#include "gui/python/python_console_widget.h"
#include "gui/python/python_editor.h"
#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_view.h"
#include "gui/settings/settings_manager.h"
#include "gui/settings/settings_items/settings_item_dropdown.h"
#include "gui/settings/settings_items/settings_item_keybind.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "hal_core/plugin_system/gui_extension_interface.h"
/*
#include "gui/content_layout_area/content_layout_area.h"
#include "gui/graph_widget/graph_graphics_view.h"
#include "gui/gui_utils/graphics.h"
#include "hal_core/plugin_system/plugin_interface_base.h"
#include "hal_core/plugin_system/plugin_manager.h"
*/
#include <QGraphicsScene>
#include <QGraphicsView>

namespace hal
{

    ExternalContent* ExternalContent::inst = nullptr;

    ExternalContent* ExternalContent::instance()
    {
        if (!inst)
            inst = new ExternalContent;
        return inst;
    }

    void ExternalContent::removePlugin(const QString& pluginName)
    {
        auto it = begin();
        while (it != end())
            if ((*it)->name() == pluginName)
                it = erase(it);
            else
                ++it;

        for (;;)
        {
            auto itw = openWidgets.find(pluginName);
            if (itw == openWidgets.end()) break;

            itw.value()->remove();
            itw.value()->deleteLater();
            openWidgets.erase(itw);
        }
    }

    ExternalContentWidget::ExternalContentWidget(const QString& pluginName, const QString& windowName, QWidget* parent)
        : ContentWidget(windowName,parent), mPluginName(pluginName)
    {
        ExternalContent::instance()->openWidgets.insert(mPluginName,this);
    }

    ExternalContentWidget::~ExternalContentWidget() {;}

    SettingsItemDropdown* ContentManager::sSettingSortMechanism;
    SettingsItemKeybind* ContentManager::sSettingSearch;
    SettingsItemKeybind* ContentManager::sSettingDeleteItem;
    bool ContentManager::sSettingsInitialized = initializeSettings();
    bool ContentManager::initializeSettings()
    {
        sSettingSortMechanism = new SettingsItemDropdown(
            "Sort Mechanism", "navigation/sort_mechanism", gui_utility::mSortMechanism::lexical, "eXpert Settings:Miscellaneous", "Specifies the sort mechanism used in every list ");
        sSettingSortMechanism->setValueNames<gui_utility::mSortMechanism>();

        sSettingSearch =
            new SettingsItemKeybind("Search",
                                    "keybinds/searchbar_toggle",
                                    QKeySequence("Ctrl+F"),
                                    "Keybindings:Global",
                                    "Keybind for toggeling the searchbar in widgets where available (Selection Details Widget, Modules Widget, Python Editor, Views Widget, Grouping Widget).");


        sSettingDeleteItem =
            new SettingsItemKeybind("Delete Item",
                                    "keybinds/item_delete",
                                    QKeySequence("Del"),
                                    "Keybindings:Global",
                                    "Keybind for deleting the focused Item.");
        return true;
    }

    ContentManager::ContentManager(MainWindow* parent) : QObject(parent), mMainWindow(parent),
        mPythonConsoleWidget(nullptr),
        mPythonWidget(nullptr),
        mGraphTabWidget(nullptr),
        mModuleWidget(nullptr),
        mContextManagerWidget(nullptr),
        mGroupingManagerWidget(nullptr),
        mSelectionDetailsWidget(nullptr),
        mLoggerWidget(nullptr),
        mContextSerializer(nullptr)
    {


        // has to be created this early in order to receive deserialization by the core signals
        mPythonWidget = new PythonEditor();

        connect(FileManager::get_instance(), &FileManager::fileOpened, this, &ContentManager::handleOpenDocument);
        //connect(FileManager::get_instance(), &FileManager::fileChanged, this, &ContentManager::handleFilsystemDocChanged);
    }

    ContentManager::~ContentManager()
    {
    }

    void ContentManager::deleteContent()
    {
        auto it = ExternalContent::instance()->openWidgets.begin();
        while (it != ExternalContent::instance()->openWidgets.end())
        {
            it.value()->deleteLater();
            it = ExternalContent::instance()->openWidgets.erase(it);
        }

        //mPythonWidget = nullptr; DONT DO THIS PYTHON_WIDGET IS CREATED IN THE CONSTRUCTOR FOR SOME REASON

        mPythonConsoleWidget->deleteLater();
        mPythonConsoleWidget    = nullptr;
        mGraphTabWidget->deleteLater();
        mGraphTabWidget         = nullptr;
        mModuleWidget->deleteLater();
        mModuleWidget           = nullptr;
        mContextManagerWidget->deleteLater();
        mContextManagerWidget   = nullptr;
        mGroupingManagerWidget->deleteLater();
        mGroupingManagerWidget  = nullptr;
        mSelectionDetailsWidget->deleteLater();
        mSelectionDetailsWidget = nullptr;
        mLoggerWidget->deleteLater();
        mLoggerWidget           = nullptr;
        if (mContextSerializer) delete mContextSerializer;
    }

    PythonEditor* ContentManager::getPythonEditorWidget()
    {
        return mPythonWidget;
    }

    GraphTabWidget* ContentManager::getGraphTabWidget()
    {
        return mGraphTabWidget;
    }

    SelectionDetailsWidget* ContentManager::getSelectionDetailsWidget()
    {
        return mSelectionDetailsWidget;
    }

    GroupingManagerWidget* ContentManager::getGroupingManagerWidget()
    {
        return mGroupingManagerWidget;
    }

    ModuleWidget* ContentManager::getModuleWidget()
    {
        return mModuleWidget;
    }

    ContextManagerWidget* ContentManager::getContextManagerWidget()
    {
        return mContextManagerWidget;
    }

    void ContentManager::handleOpenDocument(const QString& fileName)
    {
        int rightIndex = 1;

        mGraphTabWidget         = new GraphTabWidget;
        mModuleWidget           = new ModuleWidget;
        mContextManagerWidget   = new ContextManagerWidget(mGraphTabWidget);
        mGroupingManagerWidget  = new GroupingManagerWidget;
        mSelectionDetailsWidget = new SelectionDetailsWidget;
        mLoggerWidget           = new LoggerWidget;
        mPythonConsoleWidget    = new PythonConsoleWidget;

        QList<ContentWidgetPlacement> contentPlaceList;
        contentPlaceList.append({mGraphTabWidget,         0, ContentLayout::Position::Center, true});
        contentPlaceList.append({mModuleWidget,           0, ContentLayout::Position::Left,   true});
        contentPlaceList.append({mContextManagerWidget,   1, ContentLayout::Position::Left,   true});
        contentPlaceList.append({mGroupingManagerWidget,  2, ContentLayout::Position::Left,   true});
        contentPlaceList.append({mSelectionDetailsWidget, 0, ContentLayout::Position::Bottom, true});
        contentPlaceList.append({mLoggerWidget,           1, ContentLayout::Position::Bottom, false});
        contentPlaceList.append({mPythonConsoleWidget,    2, ContentLayout::Position::Bottom, true});
        contentPlaceList.append({mPythonWidget,           0, ContentLayout::Position::Right,  true});

        for (ContentFactory* cf : *ExternalContent::instance())
        {
            ContentWidget* cw = cf->contentFactory();
            cw->restoreFromProject();
            contentPlaceList.append({cw, rightIndex++, ContentLayout::Position::Right, true});
        }

        for (ContentWidgetPlacement& cwp : contentPlaceList)
        {
            ContentWidgetPlacement settingPlacement = SettingsManager::instance()->widgetPlacement(cwp.widget);
            if (settingPlacement.index >= 0)
                cwp = settingPlacement;
        }

        int placeIndex = 0;
        while (!contentPlaceList.isEmpty())
        {
            auto it = contentPlaceList.begin();
            while (it != contentPlaceList.end())
            {
                if (it->index == placeIndex)
                {
                    mMainWindow->addContent(it->widget, it->index, it->anchorPos);
                    if (it->visible) it->widget->open();

                    SettingsManager::instance()->widgetDetach(it->widget);
                    it = contentPlaceList.erase(it);
                }
                else
                    ++it;
            }
            ++placeIndex;
        }

        setWindowTitle(fileName);

#ifdef HAL_STUDY
        //log_info("gui", "HAL_STUDY activated");
        mSpecialLogContentManager = new SpecialLogContentManager(mMainWindow, mPythonWidget);
        mSpecialLogContentManager->startLogging(60000);
#endif
        connect(sSettingSortMechanism, &SettingsItemDropdown::intChanged, mSelectionDetailsWidget, [this](int value) {
            mSelectionDetailsWidget->selectionTreeView()->proxyModel()->setSortMechanism(gui_utility::mSortMechanism(value));
        });

        connect(sSettingSortMechanism, &SettingsItemDropdown::intChanged, mModuleWidget, [this](int value) { mModuleWidget->proxyModel()->setSortMechanism(gui_utility::mSortMechanism(value)); });

        connect(sSettingSortMechanism, &SettingsItemDropdown::intChanged, mGroupingManagerWidget, [this](int value) {
            mGroupingManagerWidget->getProxyModel()->setSortMechanism(gui_utility::mSortMechanism(value));
        });

        sSettingSortMechanism->intChanged(sSettingSortMechanism->value().toInt());

        connect(sSettingSearch, &SettingsItemKeybind::keySequenceChanged, mContextManagerWidget, &ContextManagerWidget::handleSearchKeysequenceChanged);
        connect(sSettingSearch, &SettingsItemKeybind::keySequenceChanged, mModuleWidget, &ContextManagerWidget::handleSearchKeysequenceChanged);
        connect(sSettingSearch, &SettingsItemKeybind::keySequenceChanged, mPythonWidget, &ContextManagerWidget::handleSearchKeysequenceChanged);
        connect(sSettingSearch, &SettingsItemKeybind::keySequenceChanged, mGroupingManagerWidget, &ContextManagerWidget::handleSearchKeysequenceChanged);
        connect(sSettingSearch, &SettingsItemKeybind::keySequenceChanged, mSelectionDetailsWidget, &ContextManagerWidget::handleSearchKeysequenceChanged);

        sSettingSearch->keySequenceChanged(sSettingSearch->value().toString());

        GraphContext* selectedContext = nullptr;

//        mContextManagerWidget->selectViewContext(new_context);

        // try to restore from project directory
        mContextSerializer = new GraphContextSerializer;
        if (mContextSerializer->restore())
        {
            selectedContext = mContextSerializer->selectedContext();
        }
        else
        {
            selectedContext = topModuleContextFactory();
            gGraphContextManager->restoreFromFile(fileName + "v");
        }

        if (selectedContext)
            mContextManagerWidget->selectViewContext(selectedContext);
        mContextManagerWidget->handleOpenContextClicked();

        for (GuiExtensionInterface* geif : GuiPluginManager::getGuiExtensions().values())
            geif->netlist_loaded(gNetlist);

        mMainWindow->layoutArea()->restoreSplitter();
    }

    void ContentManager::addExternalWidget(ContentFactory* factory)
    {
        ContentWidget* cw = factory->contentFactory();
        mMainWindow->addContent(cw, 1, ContentLayout::Position::Right);
        cw->open();
        cw->restoreFromProject();
    }

    GraphContext* ContentManager::topModuleContextFactory()
    {
        Module* top_module        = gNetlist->get_top_module();
        if (!top_module) return nullptr;
        QString context_name      = QString::fromStdString(top_module->get_name()) + " (ID: " + QString::number(top_module->get_id()) + ")";
        GraphContext* retval      = gGraphContextManager->createNewContext(context_name);
        retval->add({top_module->get_id()}, {});
        retval->setExclusiveModuleId(top_module->get_id());
        retval->setDirty(false);
        return retval;
    }

    void ContentManager::setWindowTitle(const QString& filename)
    {
        mWindowTitle = "HAL - " + QString::fromStdString(std::filesystem::path(filename.toStdString()).stem().string());
        mMainWindow->setWindowTitle(mWindowTitle);
    }
}    // namespace hal
