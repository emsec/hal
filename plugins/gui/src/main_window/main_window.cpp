#include "gui/main_window/main_window.h"

#include "gui/action/action.h"
#include "gui/content_manager/content_manager.h"
#include "gui/docking_system/dock_bar.h"
#include "gui/export/export_project_dialog.h"
#include "gui/export/export_registered_format.h"
#include "gui/export/import_project_dialog.h"
#include "gui/file_manager/file_manager.h"
#include "gui/file_manager/project_dir_dialog.h"
#include "gui/gatelibrary_management/gatelibrary_management_dialog.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui/gui_def.h"
#include "gui/gui_globals.h"
#include "gui/logger/logger_widget.h"
#include "gui/main_window/about_dialog.h"
#include "gui/main_window/plugin_parameter_dialog.h"
#include "gui/plugin_relay/gui_plugin_manager.h"
#include "gui/python/python_editor.h"
#include "gui/settings/settings_items/settings_item_checkbox.h"
#include "gui/settings/settings_items/settings_item_dropdown.h"
#include "gui/settings/settings_items/settings_item_keybind.h"
#include "gui/settings/settings_manager.h"
#include "gui/user_action/action_open_netlist_file.h"
#include "gui/welcome_screen/welcome_screen.h"
#include "hal_core/defines.h"
#include "hal_core/netlist/event_system/event_log.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "hal_core/netlist/project_manager.h"
#include "hal_core/plugin_system/gui_extension_interface.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/project_directory.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFuture>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QRegExp>
#include <QShortcut>
#include <QStringList>
#include <QtConcurrent>

namespace hal
{
    const char* MSG_PROJECT_ALREADY_OPEN = "You are already working on a HAL project. Close current project first.";

    SettingsItemDropdown* MainWindow::sSettingStyle = nullptr;

    MainWindow::MainWindow(QWidget* parent) : QWidget(parent)
    {
        ensurePolished();    // ADD REPOLISH METHOD
        connect(FileManager::get_instance(), &FileManager::fileOpened, this, &MainWindow::handleFileOpened);
        connect(FileManager::get_instance(), &FileManager::projectOpened, this, &MainWindow::handleProjectOpened);

        mLayout = new QVBoxLayout(this);
        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);

        mMenuBar = new QMenuBar();
        mMenuBar->setNativeMenuBar(true);
        mMenuBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        mLayout->addWidget(mMenuBar);

        mToolBarLayout = new QHBoxLayout();
        mToolBarLayout->setContentsMargins(0, 0, 0, 0);
        mToolBarLayout->setSpacing(0);
        mLayout->addLayout(mToolBarLayout);

        mLeftToolBar = new QToolBar();
        mLeftToolBar->setObjectName("Toolbar");
        mLeftToolBar->setMovable(false);
        mLeftToolBar->setFloatable(false);
        mLeftToolBar->setIconSize(QSize(18, 18));
        mLeftToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
        mToolBarLayout->addWidget(mLeftToolBar);

        mRightToolBar = new QToolBar();
        mRightToolBar->setObjectName("Toolbar");
        mRightToolBar->setMovable(false);
        mRightToolBar->setFloatable(false);
        mRightToolBar->setIconSize(QSize(18, 18));
        mRightToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
        mRightToolBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mToolBarLayout->addWidget(mRightToolBar);

        mStackedWidget = new QStackedWidget();
        mLayout->addWidget(mStackedWidget);

        mSettings = new MainSettingsWidget;
        mStackedWidget->addWidget(mSettings);

        mPluginManager = new GuiPluginManager(this);
        connect(mPluginManager,&GuiPluginManager::backToNetlist,this,&MainWindow::closePluginManager);
        mStackedWidget->addWidget(mPluginManager);

        mLayoutArea = new ContentLayoutArea();
        mStackedWidget->addWidget(mLayoutArea);

        //    mContainer = new QWidget();
        //    mStackedWidget->addWidget(mContainer);

        //    mContainerLayout = new QVBoxLayout(mContainer);

        //    mContainerLayout->setContentsMargins(0, 0, 0, 0);
        //    mContainerLayout->setSpacing(0);

        //    m_tool_bar = new QToolBar();
        //    m_tool_bar->setMovable(false);
        //    m_tool_bar->setFloatable(false);
        //    m_tool_bar->setIconSize(QSize(18, 18));
        //    m_tool_bar->setToolButtonStyle(Qt::ToolButtonIconOnly);
        //    mContainerLayout->addWidget(m_tool_bar);

        //    mLayoutArea = new hal_content_layout_area();
        //    mContainerLayout->addWidget(mLayoutArea);

        //welcome screen + other screens (?)

        mWelcomeScreen = new WelcomeScreen();
        mStackedWidget->addWidget(mWelcomeScreen);
        mStackedWidget->setCurrentWidget(mWelcomeScreen);

        setLocale(QLocale(QLocale::English, QLocale::UnitedStates));

        mActionNew                = new Action(this);
        mActionOpenProject        = new Action(this);
        mActionImportNetlist      = new Action(this);
        mActionSave               = new Action(this);
        mActionSaveAs             = new Action(this);
        mActionExportProject      = new Action(this);
        mActionImportProject      = new Action(this);
//        mActionGateLibraryManager = new Action(this);
        mActionAbout              = new Action(this);

        mActionStartRecording     = new Action(this);
        mActionStopRecording      = new Action(this);
        mActionPlayMacro          = new Action(this);
        mActionUndo               = new Action(this);

        mActionSettings           = new Action(this);
        mActionPlugins            = new Action(this);
        mActionClose              = new Action(this);
        mActionQuit               = new Action(this);

        //    //mOpenIconStyle = "all->#fcfcb0";
        //    //mOpenIconStyle = "all->#f2e4a4";
        //    mOpenIconStyle = "all->#3192C5";
        //    mOpenIconPath  = ":/icons/folder";

        //    mSaveIconStyle = "all->#3192C5";
        //    mSaveIconPath  = ":/icons/folder-down";

        //    //mScheduleIconStyle = "all->#08d320";
        //    //mScheduleIconStyle = "all->#05c61c";
        //    mScheduleIconStyle = "all->#2BAD4A";
        //    mScheduleIconPath  = ":/icons/list";

        //    //mRunIconStyle = "all->#05c61c";
        //    mRunIconStyle = "all->#2BAD4A";
        //    mRunIconPath  = ":/icons/run";

        //    mContentIconStyle = "all->#AFB1B3";
        //    mContentIconPath  = ":/icons/content";

        //    mSettingsIconStyle = "all->#AFB1B3";
        //    mSettingsIconPath  = ":/icons/settings";

        setWindowIcon(gui_utility::getStyledSvgIcon(mHalIconStyle, mHalIconPath));

        mActionNew->setIcon(gui_utility::getStyledSvgIcon(mNewFileIconStyle, mNewFileIconPath));
        mActionOpenProject->setIcon(gui_utility::getStyledSvgIcon(mOpenProjIconStyle, mOpenProjIconPath));
        mActionImportNetlist->setIcon(gui_utility::getStyledSvgIcon(mOpenFileIconStyle, mOpenFileIconPath));
        mActionImportProject->setIcon(gui_utility::getStyledSvgIcon(mOpenProjIconStyle, mOpenProjIconPath));
        mActionSave->setIcon(gui_utility::getStyledSvgIcon(mSaveIconStyle, mSaveIconPath));
        mActionSaveAs->setIcon(gui_utility::getStyledSvgIcon(mSaveAsIconStyle, mSaveAsIconPath));
        mActionClose->setIcon(gui_utility::getStyledSvgIcon(mCloseIconStyle, mCloseIconPath));
        mActionQuit->setIcon(gui_utility::getStyledSvgIcon(mQuitIconStyle, mQuitIconPath));
//        mActionGateLibraryManager->setIcon(gui_utility::getStyledSvgIcon(mSaveAsIconStyle, mSaveAsIconPath));
        mActionUndo->setIcon(gui_utility::getStyledSvgIcon(mUndoIconStyle, mUndoIconPath));
        mActionSettings->setIcon(gui_utility::getStyledSvgIcon(mSettingsIconStyle, mSettingsIconPath));
        mActionPlugins->setIcon(gui_utility::getStyledSvgIcon(mPluginsIconStyle, mPluginsIconPath));

        mMenuFile  = new QMenu(mMenuBar);
        mMenuEdit  = new QMenu(mMenuBar);
        mMenuMacro = new QMenu(mMenuBar);
        mMenuHelp  = new QMenu(mMenuBar);

        mMenuBar->addAction(mMenuFile->menuAction());
        mMenuBar->addAction(mMenuEdit->menuAction());
        mMenuBar->addAction(mMenuMacro->menuAction());
        mMenuBar->addAction(mMenuHelp->menuAction());
        mMenuFile->addAction(mActionNew);
        mMenuFile->addAction(mActionOpenProject);
        mMenuFile->addAction(mActionClose);
        mMenuFile->addAction(mActionSave);
        mMenuFile->addAction(mActionSaveAs);
//        mMenuFile->addAction(mActionGateLibraryManager);

        QMenu* menuImport = new QMenu("Import …", this);
        menuImport->addAction(mActionImportNetlist);
        menuImport->addAction(mActionImportProject);

        QMenu* menuExport = new QMenu("Export …", this);
        bool hasExporter = false;
        for (auto it : netlist_writer_manager::get_writer_extensions())
        {
            if (it.second.empty()) continue; // no extensions registered

            QString label = QString::fromStdString(it.first);
            QRegExp re("Default (.*) Writer", Qt::CaseInsensitive);
            QString txt = (re.indexIn(label) < 0)
                    ? label.remove(QChar(':'))
                    : QString("Export as ") + re.cap(1);

            QStringList extensions;
            extensions.append(txt);
            for (std::string ex : it.second)
                extensions.append(QString::fromStdString(ex));

            hasExporter = true;
            Action* action = new Action(txt, this);
            action->setData(extensions);
            connect(action, &QAction::triggered, this, &MainWindow::handleActionExport);
            menuExport->addAction(action);
        }
        if (hasExporter) mMenuFile->addSeparator();
        mActionExportProject->setDisabled(true);
        menuExport->addAction(mActionExportProject);

        mMenuFile->addMenu(menuImport);
        mMenuFile->addMenu(menuExport);
        mMenuFile->addSeparator();
        mMenuFile->addAction(mActionQuit);

        SettingsItemCheckbox* evlogSetting =  new SettingsItemCheckbox(
                    "Netlist event log",
                    "debug/event_log",
                    false,
                    "eXpert Settings:Debug",
                    "Specifies whether each netlist event gets dumped to event log. Might generate a lot of output thus slowing down hal system."
                );
        event_log::enable_event_log(evlogSetting->value().toBool());
        connect(evlogSetting,&SettingsItemCheckbox::boolChanged,this,&MainWindow::handleEventLogEnabled);

        mMenuEdit->addAction(mActionUndo);
        mMenuEdit->addSeparator();
        mMenuEdit->addAction(mActionSettings);
        mMenuMacro->addAction(mActionStartRecording);
        mMenuMacro->addAction(mActionStopRecording);
        mMenuMacro->addSeparator();
        mMenuMacro->addAction(mActionPlayMacro);
        mMenuHelp->addAction(mActionAbout);
        mMenuHelp->addSeparator();
        mMenuHelp->addAction(mActionPlugins);
        mLeftToolBar->addAction(mActionNew);
        mLeftToolBar->addAction(mActionOpenProject);
        mLeftToolBar->addAction(mActionSave);
        mLeftToolBar->addAction(mActionSaveAs);
        mLeftToolBar->addAction(mActionUndo);
        mRightToolBar->addAction(mActionPlugins);
        mRightToolBar->addAction(mActionSettings);

        mActionStartRecording->setText("Start recording");
        mActionStopRecording->setText("Stop recording");
        mActionPlayMacro->setText("Play macro");

        mActionStartRecording->setEnabled(true);
        mActionStopRecording->setEnabled(false);
        mActionPlayMacro->setEnabled(true);

        setWindowTitle("HAL");
        mActionNew->setText("New Project");
        mActionOpenProject->setText("Open Project");
        mActionSave->setText("Save");
        mActionSaveAs->setText("Save As");
        mActionImportNetlist->setText("Import Netlist");
        mActionImportProject->setText("Import Project");
        mActionExportProject->setText("Export Project");
//        mActionGateLibraryManager->setText("Gate Library Manager");
        mActionUndo->setText("Undo");
        mActionAbout->setText("About");
        mActionSettings->setText("Settings");
        mActionPlugins->setText("Plugin Manager");
        mActionClose->setText("Close Project");
        mActionQuit->setText("Quit");
        mMenuFile->setTitle("File");
        mMenuEdit->setTitle("Edit");
        mMenuMacro->setTitle("Macro");
        mMenuHelp->setTitle("Help");

        gPythonContext = new PythonContext(this);

        gContentManager = new ContentManager(this);
        gCommentManager = new CommentManager(this);

        mSettingCreateFile = new SettingsItemKeybind(
            "HAL Shortcut 'Create Empty Netlist'", "keybinds/project_create_file", QKeySequence("Ctrl+N"), "Keybindings:Global", "Keybind for creating a new and empty netlist in HAL.");

        mSettingOpenFile = new SettingsItemKeybind("HAL Shortcut 'Open File'", "keybinds/project_open_file", QKeySequence("Ctrl+O"), "Keybindings:Global", "Keybind for opening a new File in HAL.");

        mSettingSaveFile =
            new SettingsItemKeybind("HAL Shortcut 'Save File'", "keybinds/project_save_file", QKeySequence("Ctrl+S"), "Keybindings:Global", "Keybind for saving the currently opened file.");

        mSettingUndoLast = new SettingsItemKeybind("Undo Last Action", "keybinds/action_undo", QKeySequence("Ctrl+Z"), "Keybindings:Global", "Keybind for having last user interaction undone.");

        QShortcut* shortCutNewFile  = new QShortcut(mSettingCreateFile->value().toString(), this);
        QShortcut* shortCutOpenFile = new QShortcut(mSettingOpenFile->value().toString(), this);
        QShortcut* shortCutSaveFile = new QShortcut(mSettingSaveFile->value().toString(), this);
        QShortcut* shortCutUndoLast = new QShortcut(mSettingUndoLast->value().toString(), this);

        connect(mSettingCreateFile, &SettingsItemKeybind::keySequenceChanged, shortCutNewFile, &QShortcut::setKey);
        connect(mSettingOpenFile, &SettingsItemKeybind::keySequenceChanged, shortCutOpenFile, &QShortcut::setKey);
        connect(mSettingSaveFile, &SettingsItemKeybind::keySequenceChanged, shortCutSaveFile, &QShortcut::setKey);
        connect(mSettingUndoLast, &SettingsItemKeybind::keySequenceChanged, shortCutUndoLast, &QShortcut::setKey);

        connect(shortCutNewFile, &QShortcut::activated, mActionNew, &QAction::trigger);
        connect(shortCutOpenFile, &QShortcut::activated, mActionOpenProject, &QAction::trigger);
        connect(shortCutSaveFile, &QShortcut::activated, mActionSave, &QAction::trigger);
        connect(shortCutUndoLast, &QShortcut::activated, mActionUndo, &QAction::trigger);

        connect(mActionNew, &Action::triggered, this, &MainWindow::handleActionNew);
        connect(mActionOpenProject, &Action::triggered, this, &MainWindow::handleActionOpenProject);
        connect(mActionImportNetlist, &Action::triggered, this, &MainWindow::handleActionImportNetlist);
        connect(mActionAbout, &Action::triggered, this, &MainWindow::handleActionAbout);
        connect(mActionSettings, &Action::triggered, this, &MainWindow::openSettings);
        connect(mActionPlugins, &Action::triggered, this, &MainWindow::openPluginManager);
        connect(mSettings, &MainSettingsWidget::close, this, &MainWindow::closeSettings);
        connect(mActionSave, &Action::triggered, this, &MainWindow::handleSaveTriggered);
        connect(mActionSaveAs, &Action::triggered, this, &MainWindow::handleSaveAsTriggered);
        connect(mActionExportProject, &Action::triggered, this, &MainWindow::handleExportProjectTriggered);
        connect(mActionImportProject, &Action::triggered, this, &MainWindow::handleImportProjectTriggered);
//        connect(mActionGateLibraryManager, &Action::triggered, this, &MainWindow::handleActionGatelibraryManager);
        connect(mActionClose, &Action::triggered, this, &MainWindow::handleActionCloseFile);
        connect(mActionQuit, &Action::triggered, this, &MainWindow::onActionQuitTriggered);

        connect(mActionStartRecording, &Action::triggered, this, &MainWindow::handleActionStartRecording);
        connect(mActionStopRecording, &Action::triggered, this, &MainWindow::handleActionStopRecording);
        connect(mActionPlayMacro, &Action::triggered, this, &MainWindow::handleActionPlayMacro);
        connect(mActionUndo, &Action::triggered, this, &MainWindow::handleActionUndo);

        connect(UserActionManager::instance(), &UserActionManager::canUndoLastAction, this, &MainWindow::enableUndo);
        connect(sSettingStyle, &SettingsItemDropdown::intChanged, this, &MainWindow::reloadStylsheet);
        enableUndo(false);

        restoreState();
    }

    void MainWindow::reloadStylsheet(int istyle)
    {
        QString styleSheetToOpen;

        switch (istyle)
        {
        case StyleSheetOption::Dark:
            styleSheetToOpen = ":/style/dark";
            break;
        case StyleSheetOption::Light:
            styleSheetToOpen = ":/style/light";
            break;
        default:
            return;
        }
        QFile stylesheet(styleSheetToOpen);
        stylesheet.open(QFile::ReadOnly);
        qApp->setStyleSheet(QString(stylesheet.readAll()));
    }

    QString MainWindow::halIconPath() const
    {
        return mHalIconPath;
    }

    QString MainWindow::halIconStyle() const
    {
        return mHalIconStyle;
    }

    QString MainWindow::newFileIconPath() const
    {
        return mNewFileIconPath;
    }

    QString MainWindow::newFileIconStyle() const
    {
        return mNewFileIconStyle;
    }

    QString MainWindow::openProjIconPath() const
    {
        return mOpenProjIconPath;
    }

    QString MainWindow::openProjIconStyle() const
    {
        return mOpenProjIconStyle;
    }

    QString MainWindow::openFileIconPath() const
    {
        return mOpenFileIconPath;
    }

    QString MainWindow::openFileIconStyle() const
    {
        return mOpenFileIconStyle;
    }

    QString MainWindow::saveIconPath() const
    {
        return mSaveIconPath;
    }

    QString MainWindow::saveIconStyle() const
    {
        return mSaveIconStyle;
    }

    QString MainWindow::saveAsIconPath() const
    {
        return mSaveAsIconPath;
    }

    QString MainWindow::saveAsIconStyle() const
    {
        return mSaveAsIconStyle;
    }

    QString MainWindow::closeIconPath() const
    {
        return mCloseIconPath;
    }

    QString MainWindow::closeIconStyle() const
    {
        return mCloseIconStyle;
    }

    QString MainWindow::quitIconPath() const
    {
        return mQuitIconPath;
    }

    QString MainWindow::quitIconStyle() const
    {
        return mQuitIconStyle;
    }

    QString MainWindow::settingsIconPath() const
    {
        return mSettingsIconPath;
    }

    QString MainWindow::settingsIconStyle() const
    {
        return mSettingsIconStyle;
    }

    QString MainWindow::pluginsIconPath() const
    {
        return mPluginsIconPath;
    }

    QString MainWindow::pluginsIconStyle() const
    {
        return mPluginsIconStyle;
    }

    QString MainWindow::undoIconPath() const
    {
        return mUndoIconPath;
    }

    QString MainWindow::undoIconStyle() const
    {
        return mUndoIconStyle;
    }

    void MainWindow::setHalIconPath(const QString& path)
    {
        mHalIconPath = path;
    }

    void MainWindow::setHalIconStyle(const QString& style)
    {
        mHalIconStyle = style;
    }

    void MainWindow::setNewFileIconPath(const QString& path)
    {
        mNewFileIconPath = path;
    }

    void MainWindow::setNewFileIconStyle(const QString& style)
    {
        mNewFileIconStyle = style;
    }

    void MainWindow::setOpenFileIconPath(const QString& path)
    {
        mOpenFileIconPath = path;
    }

    void MainWindow::setOpenFileIconStyle(const QString& style)
    {
        mOpenFileIconStyle = style;
    }

    void MainWindow::setOpenProjIconPath(const QString& path)
    {
        mOpenProjIconPath = path;
    }

    void MainWindow::setOpenProjIconStyle(const QString& style)
    {
        mOpenProjIconStyle = style;
    }

    void MainWindow::setSaveIconPath(const QString& path)
    {
        mSaveIconPath = path;
    }

    void MainWindow::setSaveIconStyle(const QString& style)
    {
        mSaveIconStyle = style;
    }

    void MainWindow::setSaveAsIconPath(const QString& path)
    {
        mSaveAsIconPath = path;
    }

    void MainWindow::setSaveAsIconStyle(const QString& style)
    {
        mSaveAsIconStyle = style;
    }

    void MainWindow::setCloseIconPath(const QString& path)
    {
        mCloseIconPath = path;
    }

    void MainWindow::setCloseIconStyle(const QString& style)
    {
        mCloseIconStyle = style;
    }

    void MainWindow::setQuitIconPath(const QString& path)
    {
        mQuitIconPath = path;
    }

    void MainWindow::setQuitIconStyle(const QString& style)
    {
        mQuitIconStyle = style;
    }

    void MainWindow::setSettingsIconPath(const QString& path)
    {
        mSettingsIconPath = path;
    }

    void MainWindow::setSettingsIconStyle(const QString& style)
    {
        mSettingsIconStyle = style;
    }

    void MainWindow::setPluginsIconPath(const QString& path)
    {
        mPluginsIconPath = path;
    }

    void MainWindow::setPluginsIconStyle(const QString& style)
    {
        mPluginsIconStyle = style;
    }

    void MainWindow::setUndoIconPath(const QString& path)
    {
        mUndoIconPath = path;
    }

    void MainWindow::setUndoIconStyle(const QString& style)
    {
        mUndoIconStyle = style;
    }

    QString MainWindow::disabledIconStyle() const
    {
        return mDisabledIconStyle;
    }

    void MainWindow::setDisabledIconStyle(const QString& style)
    {
        mDisabledIconStyle = style;
    }

    void MainWindow::addContent(ContentWidget* widget, int index, content_anchor anchor)
    {
        mLayoutArea->addContent(widget, index, anchor);
    }

    void MainWindow::removeContent(ContentWidget* widget)
    {
        Q_UNUSED(widget)
        // IMPLEMENT
    }

    void MainWindow::clear()
    {
        mLayoutArea->clear();
    }

    void MainWindow::handleEventLogEnabled(bool enable)
    {
        event_log::enable_event_log(enable);
    }

    extern void runMain(const QString fileName, const QList<QString> plugins);

    void MainWindow::onActionCloseDocumentTriggered()
    {
        //mLayoutArea->removeContent();
    }

    void MainWindow::openSettings()
    {
        if (mStackedWidget->currentWidget() == mSettings)
            return; //nothing todo, already open

        mSettings->activate();
        mStackedWidget->setCurrentWidget(mSettings);
    }

    void MainWindow::closeSettings()
    {
        if (!mSettings->handleAboutToClose())
            return;
        if (FileManager::get_instance()->fileOpen())
            mStackedWidget->setCurrentWidget(mLayoutArea);
        else
            mStackedWidget->setCurrentWidget(mWelcomeScreen);
    }

    void MainWindow::openPluginManager()
    {
        mPluginManager->repolish();
        if (mStackedWidget->currentWidget() == mPluginManager)
            return; //nothing todo, already open

        if (mStackedWidget->currentWidget() == mSettings)
        {
            if (!mSettings->handleAboutToClose())
                return;
        }
        mStackedWidget->setCurrentWidget(mPluginManager);
    }

    void MainWindow::closePluginManager(const QString &invokeGui)
    {
        bool isFileOpen = FileManager::get_instance()->fileOpen();
        if (isFileOpen)
            mStackedWidget->setCurrentWidget(mLayoutArea);
        else
            mStackedWidget->setCurrentWidget(mWelcomeScreen);
        if (invokeGui.isEmpty() || !isFileOpen) return;
        GuiExtensionInterface* geif = GuiPluginManager::getGuiExtensions().value(invokeGui);
        if (!geif) return;
        PluginParameterDialog ppd(invokeGui,geif,this);
        ppd.exec();
    }

    void MainWindow::handleActionNew()
    {
        if (gNetlist != nullptr)
        {
            QMessageBox msgBox;
            msgBox.setText("Error: New Project");
            msgBox.setInformativeText(MSG_PROJECT_ALREADY_OPEN);
            msgBox.setStyleSheet("QLabel{min-width: 600px;}");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
            return;
        }

        ActionOpenNetlistFile* act = new ActionOpenNetlistFile(ActionOpenNetlistFile::CreateNew);
        act->exec();
    }

    void MainWindow::handleActionOpenProject()
    {
        if (gNetlist != nullptr)
        {
            QMessageBox msgBox;
            msgBox.setText("Error: Open Project");
            msgBox.setInformativeText(MSG_PROJECT_ALREADY_OPEN);
            msgBox.setStyleSheet("QLabel{min-width: 600px;}");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
            return;
        }

        ProjectDirDialog pdd("Open HAL project", QDir::currentPath(), this);
        if (pdd.exec() != QDialog::Accepted) return;
        QStringList projects = pdd.selectedFiles();
        for (int inx = 0; inx < projects.size(); ++inx)
        {
            if (!QFileInfo(projects.at(inx)).suffix().isEmpty())
            {
                QMessageBox::warning(this,"Bad project directory", "HAL project directories must not have suffix (." +
                                     QFileInfo(projects.at(inx)).suffix() + ") in name");
                continue;
            }
            ActionOpenNetlistFile* act = new ActionOpenNetlistFile(ActionOpenNetlistFile::OpenProject, pdd.selectedFiles().at(inx));
            act->exec();
            break;
        }
    }

    void MainWindow::handleActionImportNetlist()
    {
        if (gNetlist != nullptr)
        {
            QMessageBox msgBox;
            msgBox.setText("Error: Import Netlist");
            msgBox.setInformativeText(MSG_PROJECT_ALREADY_OPEN);
            msgBox.setStyleSheet("QLabel{min-width: 600px;}");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
            return;
        }

        QString title = "Import Netlist";
        SupportedFileFormats sff = gPluginRelay->mGuiPluginTable->listFacFeature(FacExtensionInterface::FacNetlistParser);
        QString text  = sff.toFileDialog(true);

        // Non native dialogs does not work on macOS. Therefore do net set DontUseNativeDialog!
        QString path = QDir::currentPath();

        if (gGuiState->contains("FileDialog/Path/MainWindow"))
            path = gGuiState->value("FileDialog/Path/MainWindow").toString();

        QString fileName = QFileDialog::getOpenFileName(nullptr, title, path, text, nullptr);

        if (!fileName.isNull())
        {
            QString ext = QFileInfo(fileName).suffix();
            if (!ext.isEmpty())
                gPluginRelay->mGuiPluginTable->loadFeature(FacExtensionInterface::FacNetlistParser,ext);

            gGuiState->setValue("FileDialog/Path/MainWindow", fileName);

            ActionOpenNetlistFile* actOpenfile = new ActionOpenNetlistFile(ActionOpenNetlistFile::ImportFile, fileName);
            actOpenfile->exec();
        }
    }

    void MainWindow::handleProjectOpened(const QString& projDir, const QString& fileName)
    {
        Q_UNUSED(projDir);
        mActionExportProject->setEnabled(true);
        mActionImportProject->setDisabled(true);
        handleFileOpened(fileName);
    }

    void MainWindow::handleFileOpened(const QString& fileName)
    {
        Q_UNUSED(fileName)
        if (mStackedWidget->currentWidget() == mWelcomeScreen)
        {
            mStackedWidget->setCurrentWidget(mLayoutArea);
            mWelcomeScreen->close();
        }
        gPythonContext->updateNetlist();

//        mActionGateLibraryManager->setVisible(false);
    }

    void MainWindow::handleActionExport()
    {
        if (!gNetlist) return;
        QAction* act = static_cast<QAction*>(sender());
        if (!act || act->data().isNull()) return;

        ExportRegisteredFormat erf(act->data().toStringList());
        if (erf.queryFilename())
            erf.exportNetlist();

    }

    void MainWindow::handleActionGatelibraryManager()
    {
        GatelibraryManagementDialog dialog;
        dialog.exec();
    }

    void MainWindow::handleImportProjectTriggered()
    {
        ImportProjectDialog ipd(this);
        if (ipd.exec() == QDialog::Accepted)
        {
            if (ipd.importProject())
            {
                ActionOpenNetlistFile* act = new ActionOpenNetlistFile(ActionOpenNetlistFile::OpenProject,
                                                                       ipd.extractedProjectAbsolutePath());
                act->exec();
            }
            else
                QMessageBox::warning(this,
                                     "Import Project Failed",
                                     "Failed to extract a HAL project from selected archive file.\n"
                                     "You might want to uncompress the archive manually and try to open the project.");
        }
    }

    void MainWindow::handleExportProjectTriggered()
    {
        ExportProjectDialog epd(this);
        if (epd.exec() == QDialog::Accepted)
        {
            epd.exportProject();
        }
    }

    void MainWindow::handleSaveAsTriggered()
    {
        QString filename = saveHandler();
        if (!filename.isEmpty())
            gContentManager->setWindowTitle(filename);
    }

    void MainWindow::handleSaveTriggered()
    {
        ProjectManager* pm = ProjectManager::instance();
        if (pm->get_project_status() == ProjectManager::ProjectStatus::NONE) return;
        QString  projectDir = QString::fromStdString(pm->get_project_directory().string());
        saveHandler(projectDir);
        gContentManager->setWindowTitle(projectDir);
    }

    QString MainWindow::saveHandler(const QString &projectDir)
    {
        if (!gNetlist) return QString();

        QString saveProjectDir(projectDir);

        ProjectManager* pm = ProjectManager::instance();

        if (saveProjectDir.isEmpty())
        {
            QString title = "Save Project";
            QString filter = "HAL Directory Folder (*)";

            // Non native dialogs does not work on macOS. Therefore do net set DontUseNativeDialog!
            saveProjectDir = QFileDialog::getSaveFileName(nullptr, title, QDir::currentPath(), filter, nullptr);
            if (saveProjectDir.isEmpty()) return QString();

            QFileInfo finfo(saveProjectDir);

            if (!finfo.suffix().isEmpty())
            {
                QMessageBox::warning(this,"Save Error", "selected project directory name must not have suffix ." + finfo.suffix());
                return QString();
            }
            if (finfo.exists())
            {
                QMessageBox::warning(this,"Save Error", "folder " + saveProjectDir + " already exists");
                return QString();
            }

            if (!pm->create_project_directory(saveProjectDir.toStdString()))
            {
                QMessageBox::warning(this,"Save Error", "cannot create folder " + saveProjectDir);
                return QString();
            }
        }

        QString qNetlistPath = QString::fromStdString(pm->get_project_directory().get_default_filename(".hal"));

        if (!pm->serialize_project(gNetlist))
        {
            log_warning("gui", "error saving netlist to <" + qNetlistPath.toStdString() + ">");
            return QString();
        }

        gFileStatusManager->netlistSaved();

        FileManager* fm = FileManager::get_instance();
        fm->watchFile(qNetlistPath);
        fm->emitProjectSaved(saveProjectDir, qNetlistPath);

        return QString::fromStdString(pm->get_project_directory().string());
    }

    void MainWindow::handleActionStartRecording()
    {
        mActionStartRecording->setEnabled(false);
        mActionStopRecording->setEnabled(true);
        mActionPlayMacro->setEnabled(false);
        UserActionManager::instance()->setStartRecording();
    }

    void MainWindow::handleActionStopRecording()
    {
        UserActionManager* uam = UserActionManager::instance();
        QString macroFile;
        bool trySaveMacro = true;
        while (trySaveMacro) {
            if (uam->hasRecorded())
            {
                macroFile = QFileDialog::getSaveFileName(this, "Save macro to file", ".");
                if (!macroFile.isEmpty() && !macroFile.contains(QChar('.')))
                    macroFile += ".xml";
            }
            switch (uam->setStopRecording(macroFile))
            {
            case QMessageBox::Retry:
                break;
            case QMessageBox::Cancel:
                return;
            default:
                // Ok or Discard
                trySaveMacro = false;
                break;
            }
        }
        mActionStartRecording->setEnabled(true);
        mActionStopRecording->setEnabled(false);
        mActionPlayMacro->setEnabled(true);
    }

    void MainWindow::handleActionPlayMacro()
    {
        QString macroFile = QFileDialog::getOpenFileName(this, "Load macro file", ".", "Macro files (*.xml);;All files(*)");
        if (!macroFile.isEmpty())
            UserActionManager::instance()->playMacro(macroFile);
    }

    void MainWindow::handleActionAbout()
    {
        AboutDialog ad;
        ad.exec();
    }

    void MainWindow::handleActionUndo()
    {
        UserActionManager::instance()->undoLastAction();
    }

    void MainWindow::enableUndo(bool enable)
    {
        QString iconStyle = enable ? mUndoIconStyle : mDisabledIconStyle;
        mActionUndo->setEnabled(enable);
        mActionUndo->setIcon(gui_utility::getStyledSvgIcon(iconStyle, mUndoIconPath));
    }

    void MainWindow::handleActionCloseFile()
    {
        if (FileManager::get_instance()->fileOpen())
            tryToCloseFile();
    }

    void MainWindow::onActionQuitTriggered()
    {
        close();
    }

    void MainWindow::closeEvent(QCloseEvent* event)
    {
        if (FileManager::get_instance()->fileOpen())
        {
            if (tryToCloseFile())
                event->accept();
            else
            {
                event->ignore();
                return;
            }
        }

        saveState();
        qApp->quit();
    }

    bool MainWindow::tryToCloseFile()
    {
        if (gFileStatusManager->modifiedFilesExisting())
        {
            QMessageBox msgBox(this);
            msgBox.setStyleSheet("QLabel{min-width: 600px;}");
            auto cancelButton = msgBox.addButton("Cancel", QMessageBox::RejectRole);
            msgBox.addButton("Close Anyway", QMessageBox::ApplyRole);
            msgBox.setDefaultButton(cancelButton);
            msgBox.setInformativeText("Are you sure you want to close the application ?");

            msgBox.setText("There are unsaved modifications.");
            QString detailed_text = "The following modifications have not been saved yet:\n";
            for (const auto& s : gFileStatusManager->getUnsavedChangeDescriptors())
                detailed_text.append("   ->  " + s + "\n");
            msgBox.setDetailedText(detailed_text);

            for (const auto& button : msgBox.buttons())
            {
                // if (button->text() == "Show Details...")
                if (msgBox.buttonRole(button) == QMessageBox::ActionRole)
                {
                    button->click();
                    break;
                }
            }

            msgBox.exec();

            if (msgBox.clickedButton() == cancelButton)
                return false;
        }

        // going to close

        for (GuiExtensionInterface* geif : GuiPluginManager::getGuiExtensions().values())
            geif->netlist_about_to_close(gNetlist);

        mActionExportProject->setDisabled(true);
        mActionImportProject->setEnabled(true);

        gPythonContext->abortThreadAndWait();

        gGraphContextManager->clear();

        clear();

        gContentManager->deleteContent();
        // PYTHON ???
        gSelectionRelay->clear();
        gCommentManager->clear();

        FileManager::get_instance()->closeFile();
        setWindowTitle("HAL");

        mStackedWidget->setCurrentWidget(mWelcomeScreen);

        gNetlistRelay->reset();

//        mActionGateLibraryManager->setVisible(true);

        return true;
    }

    void MainWindow::restoreState()
    {
        QPoint pos = SettingsManager::instance()->mainWindowPosition();
        move(pos);
        QSize size = SettingsManager::instance()->mainWindowSize();
        resize(size);
        //restore state of all subwindows
        mLayoutArea->initSplitterSize(size);
    }

    void MainWindow::saveState()
    {
        SettingsManager::instance()->mainWindowSaveGeometry(pos(), size());
    }

}    // namespace hal
