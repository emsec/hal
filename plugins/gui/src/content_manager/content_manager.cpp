#include "gui/content_manager/content_manager.h"

#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/file_manager/file_manager.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/graph_widget/graph_context_manager.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui/content_layout_area/content_layout_area.h"
#include "gui/content_widget/content_widget.h"
#include "gui/docking_system/tab_widget.h"
#include "gui/graph_widget/graph_graphics_view.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui/gui_utils/graphics.h"
#include "gui/logger/logger_widget.h"
#include "gui/module_widget/module_widget.h"
#include "gui/python/python_console_widget.h"
#include "gui/python/python_editor.h"
#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui/gui_globals.h"
#include "gui/main_window/main_window.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "gui/gui_utils/special_log_content_manager.h"
#include "gui/gui_utils/sort.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_view.h"
#include "gui/grouping/grouping_proxy_model.h"
#include "gui/module_model/module_proxy_model.h"
#include "gui/settings/settings_items/settings_item_dropdown.h"
#include "gui/settings/settings_items/settings_item_keybind.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QOpenGLWidget>

namespace hal
{
    SettingsItemDropdown* ContentManager::sSettingSortMechanism;
    SettingsItemKeybind* ContentManager::sSettingSearch;
    bool ContentManager::sSettingsInitialized = initializeSettins();
    bool ContentManager::initializeSettins()
    {
        sSettingSortMechanism = new SettingsItemDropdown(
            "Sort Mechanism",
            "navigation/sort_mechanism",
            gui_utility::mSortMechanism::lexical,
            "eXpert Settings:Miscellaneous",
            "Specifies the sort mechanism used in every list "
        );
        sSettingSortMechanism->setValueNames<gui_utility::mSortMechanism>();

        sSettingSearch = new SettingsItemKeybind(
            "Search",
            "keybinds/searchbar_toggle",
            QKeySequence("Ctrl+F"),
            "Keybindings:Global",
            "Keybind for toggeling the searchbar in widgets where available (Selection Details Widget, Modules Widget, Python Editor, Views Widget, Grouping Widget)."
        );

        return true;
    }


    ContentManager::ContentManager(MainWindow* parent) : QObject(parent), mMainWindow(parent)
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
        for (auto content : mContent)
            delete content;

        mContent.clear();

        //m_python_widget = nullptr; DONT DO THIS PYTHON_WIDGET IS CREATED IN THE CONSTRUCTOR FOR SOME REASON

        mPythonConsoleWidget = nullptr;
        mGraphTabWidget = nullptr;
        mContextManagerWidget = nullptr;
        mSelectionDetailsWidget = nullptr;
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
        mGraphTabWidget = new GraphTabWidget();
        mMainWindow->addContent(mGraphTabWidget, 2, content_anchor::center);

        mModuleWidget = new ModuleWidget();
        mMainWindow->addContent(mModuleWidget, 0, content_anchor::left);
        mModuleWidget->open();

        mContextManagerWidget = new ContextManagerWidget(mGraphTabWidget);
        mMainWindow->addContent(mContextManagerWidget, 1, content_anchor::left);
        mContextManagerWidget->open();

        mGroupingManagerWidget = new GroupingManagerWidget();
        mMainWindow->addContent(mGroupingManagerWidget, 2, content_anchor::left);
        mGroupingManagerWidget->open();

        //we should probably document somewhere why we need this timer and why we have some sort of racing condition(?) here?
        //QTimer::singleShot(50, [this]() { this->mContextManagerWid->handleCreateContextClicked(); });

        //executes same code as found in 'create_context_clicked' from the context manager widget but allows to keep its method private
/*
        QTimer::singleShot(50, [this]() {

            GraphContext* new_context = nullptr;
            new_context = gGraphContextManager->createNewContext(QString::fromStdString(gNetlist->get_top_module()->get_name()));
            new_context->add({gNetlist->get_top_module()->get_id()}, {});

            mContextManagerWidget->selectViewContext(new_context);
            gGraphContextManager->restoreFromFile();
            new_context->setDirty(false);
        });
*/
        //why does this segfault without a timer?
        //GraphContext* new_context = nullptr;
        //new_context = gGraphContextManager->createNewContext(QString::fromStdString(gNetlist->get_top_module()->get_name()));
        //new_context->add({gNetlist->get_top_module()->get_id()}, {});
        //mContextManagerWid->selectViewContext(new_context);

        mSelectionDetailsWidget = new SelectionDetailsWidget();
        mMainWindow->addContent(mSelectionDetailsWidget, 0, content_anchor::bottom);
        mSelectionDetailsWidget->open();

        mLoggerWidget = new LoggerWidget();
        mMainWindow->addContent(mLoggerWidget, 1, content_anchor::bottom);

        mMainWindow->addContent(mPythonWidget, 3, content_anchor::right);
        mPythonWidget->open();

        mPythonConsoleWidget = new PythonConsoleWidget();
        mMainWindow->addContent(mPythonConsoleWidget, 5, content_anchor::bottom);
        mPythonConsoleWidget->open();

        mContent.append(mGraphTabWidget);
        mContent.append(mModuleWidget);
        mContent.append(mContextManagerWidget);
        mContent.append(mGroupingManagerWidget);
        mContent.append(mSelectionDetailsWidget);
        mContent.append(mLoggerWidget);
        //mContent.append(mPythonWidget); // DONT DO THIS PYTHON_WIDGET IS CREATED IN THE CONSTRUCTOR FOR SOME REASON
        mContent.append(mPythonConsoleWidget);

        setWindowTitle(fileName);

#ifdef HAL_STUDY
        //log_info("gui", "HAL_STUDY activated");
        mSpecialLogContentManager = new SpecialLogContentManager(mMainWindow, mPythonWidget);
        mSpecialLogContentManager->startLogging(60000);
#endif

        connect(mSelectionDetailsWidget, &SelectionDetailsWidget::focusGateClicked, mGraphTabWidget, &GraphTabWidget::handleGateFocus);
        connect(mSelectionDetailsWidget, &SelectionDetailsWidget::focusNetClicked, mGraphTabWidget, &GraphTabWidget::handleNetFocus);
        connect(mSelectionDetailsWidget, &SelectionDetailsWidget::focusModuleClicked, mGraphTabWidget, &GraphTabWidget::handleModuleFocus);

        connect(sSettingSortMechanism, &SettingsItemDropdown::intChanged, mSelectionDetailsWidget, [this](int value){
            mSelectionDetailsWidget->selectionTreeView()->proxyModel()->setSortMechanism(gui_utility::mSortMechanism(value));
        });

        connect(sSettingSortMechanism, &SettingsItemDropdown::intChanged, mModuleWidget, [this](int value){
            mModuleWidget->proxyModel()->setSortMechanism(gui_utility::mSortMechanism(value));
        });

        connect(sSettingSortMechanism, &SettingsItemDropdown::intChanged, mGroupingManagerWidget, [this](int value){
            mGroupingManagerWidget->getProxyModel()->setSortMechanism(gui_utility::mSortMechanism(value));
        });

        sSettingSortMechanism->intChanged(sSettingSortMechanism->value().toInt());

        connect(sSettingSearch, &SettingsItemKeybind::keySequenceChanged, mContextManagerWidget, &ContextManagerWidget::handleSearchKeysequenceChanged);
        connect(sSettingSearch, &SettingsItemKeybind::keySequenceChanged, mModuleWidget, &ContextManagerWidget::handleSearchKeysequenceChanged);
        connect(sSettingSearch, &SettingsItemKeybind::keySequenceChanged, mPythonWidget, &ContextManagerWidget::handleSearchKeysequenceChanged);
        connect(sSettingSearch, &SettingsItemKeybind::keySequenceChanged, mGroupingManagerWidget, &ContextManagerWidget::handleSearchKeysequenceChanged);
        connect(sSettingSearch, &SettingsItemKeybind::keySequenceChanged, mSelectionDetailsWidget, &ContextManagerWidget::handleSearchKeysequenceChanged);

        sSettingSearch->keySequenceChanged(sSettingSearch->value().toString());

        GraphContext* new_context = nullptr;
        new_context = gGraphContextManager->createNewContext(QString::fromStdString(gNetlist->get_top_module()->get_name()));
        new_context->add({gNetlist->get_top_module()->get_id()}, {});

        mContextManagerWidget->selectViewContext(new_context);
        gGraphContextManager->restoreFromFile();
        new_context->setDirty(false);

    }

    void ContentManager::setWindowTitle(const QString &filename)
    {
        mWindowTitle = "HAL - " + QString::fromStdString(std::filesystem::path(filename.toStdString()).stem().string());
        mMainWindow->setWindowTitle(mWindowTitle);
    }
}
