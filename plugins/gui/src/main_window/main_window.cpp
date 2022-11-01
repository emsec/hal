#include "gui/main_window/main_window.h"

#include "gui/action/action.h"
#include "gui/content_manager/content_manager.h"
#include "gui/docking_system/dock_bar.h"
#include "gui/export/export_registered_format.h"
#include "gui/file_manager/file_manager.h"
#include "gui/file_manager/project_dir_dialog.h"
#include "gui/gatelibrary_management/gatelibrary_management_dialog.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui/gui_def.h"
#include "gui/gui_globals.h"
#include "gui/logger/logger_widget.h"
#include "gui/main_window/about_dialog.h"
#include "gui/main_window/plugin_parameter_dialog.h"
#include "gui/python/python_editor.h"
#include "gui/settings/settings_items/settings_item_checkbox.h"
#include "gui/settings/settings_items/settings_item_dropdown.h"
#include "gui/settings/settings_items/settings_item_keybind.h"
#include "gui/settings/settings_manager.h"
#include "gui/user_action/action_open_netlist_file.h"
#include "gui/welcome_screen/welcome_screen.h"
#include "hal_core/defines.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "hal_core/utilities/log.h"
#include "hal_core/netlist/event_system/event_log.h"
#include "hal_core/netlist/project_manager.h"
#include "hal_core/utilities/project_directory.h"
#include "hal_core/plugin_system/plugin_manager.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFuture>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QtConcurrent>
#include <QRegExp>
#include <QStringList>

namespace hal
{
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
        mActionOpenProject  = new Action(this);
        mActionImport       = new Action(this);
        mActionSave               = new Action(this);
        mActionSaveAs             = new Action(this);
        mActionGateLibraryManager = new Action(this);
        mActionAbout              = new Action(this);

        mActionStartRecording = new Action(this);
        mActionStopRecording  = new Action(this);
        mActionPlayMacro      = new Action(this);
        mActionUndo           = new Action(this);

        mActionSettings = new Action(this);
        mActionClose    = new Action(this);

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
        mActionOpenProject->setIcon(gui_utility::getStyledSvgIcon(mOpenIconStyle, mOpenIconPath));
        mActionImport->setIcon(gui_utility::getStyledSvgIcon(mOpenIconStyle, mOpenIconPath));
        mActionSave->setIcon(gui_utility::getStyledSvgIcon(mSaveIconStyle, mSaveIconPath));
        mActionSaveAs->setIcon(gui_utility::getStyledSvgIcon(mSaveAsIconStyle, mSaveAsIconPath));
        mActionGateLibraryManager->setIcon(gui_utility::getStyledSvgIcon(mSaveAsIconStyle, mSaveAsIconPath));
        mActionUndo->setIcon(gui_utility::getStyledSvgIcon(mUndoIconStyle, mUndoIconPath));
        mActionSettings->setIcon(gui_utility::getStyledSvgIcon(mSettingsIconStyle, mSettingsIconPath));

        mMenuFile  = new QMenu(mMenuBar);
        mMenuEdit  = new QMenu(mMenuBar);
        mMenuMacro = new QMenu(mMenuBar);
        mMenuPlugin = new QMenu(mMenuBar);
        mMenuHelp  = new QMenu(mMenuBar);

        mMenuBar->addAction(mMenuFile->menuAction());
        mMenuBar->addAction(mMenuEdit->menuAction());
        mMenuBar->addAction(mMenuMacro->menuAction());
        mMenuBar->addAction(mMenuPlugin->menuAction());
        mMenuBar->addAction(mMenuHelp->menuAction());
        mMenuFile->addAction(mActionNew);
        mMenuFile->addAction(mActionOpenProject);
        mMenuFile->addAction(mActionImport);
        mMenuFile->addAction(mActionClose);
        mMenuFile->addAction(mActionSave);
        mMenuFile->addAction(mActionSaveAs);
        mMenuFile->addAction(mActionGateLibraryManager);

        QMenu* menuExport = nullptr;
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

            if (!menuExport) menuExport = new QMenu("Export …", this);
            Action* action = new Action(txt, this);
            action->setData(extensions);
            connect(action, &QAction::triggered, this, &MainWindow::handleActionExport);
            menuExport->addAction(action);
        }
        if (menuExport) mMenuFile->addMenu(menuExport);

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
        mLeftToolBar->addAction(mActionNew);
        mLeftToolBar->addAction(mActionOpenProject);
        mLeftToolBar->addAction(mActionSave);
        mLeftToolBar->addAction(mActionSaveAs);
        mLeftToolBar->addAction(mActionUndo);
        mRightToolBar->addAction(mActionSettings);

        mActionStartRecording->setText("Start recording");
        mActionStopRecording->setText("Stop recording");
        mActionPlayMacro->setText("Play macro");

        mActionStartRecording->setEnabled(true);
        mActionStopRecording->setEnabled(false);
        mActionPlayMacro->setEnabled(true);

        pluginMenu();
        setWindowTitle("HAL");
        mActionNew->setText("New Netlist");
        mActionOpenProject->setText("Open Project");
        mActionImport->setText("Import Netlist");
        mActionSave->setText("Save");
        mActionSaveAs->setText("Save As");
        mActionGateLibraryManager->setText("Gate Library Manager");
        mActionUndo->setText("Undo");
        mActionAbout->setText("About");
        mActionSettings->setText("Settings");
        mActionClose->setText("Close Document");
        mMenuFile->setTitle("File");
        mMenuEdit->setTitle("Edit");
        mMenuMacro->setTitle("Macro");
        mMenuPlugin->setTitle("Plugins");
        mMenuHelp->setTitle("Help");

        gPythonContext = new PythonContext(this);

        gContentManager = new ContentManager(this);

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
        connect(mActionImport, &Action::triggered, this, &MainWindow::handleActionImport);
        connect(mActionAbout, &Action::triggered, this, &MainWindow::handleActionAbout);
        connect(mActionSettings, &Action::triggered, this, &MainWindow::toggleSettings);
        connect(mSettings, &MainSettingsWidget::close, this, &MainWindow::closeSettings);
        connect(mActionSave, &Action::triggered, this, &MainWindow::handleSaveTriggered);
        connect(mActionSaveAs, &Action::triggered, this, &MainWindow::handleSaveAsTriggered);
        connect(mActionGateLibraryManager, &Action::triggered, this, &MainWindow::handleActionGatelibraryManager);
        connect(mActionClose, &Action::triggered, this, &MainWindow::handleActionCloseFile);

        connect(mActionStartRecording, &Action::triggered, this, &MainWindow::handleActionStartRecording);
        connect(mActionStopRecording, &Action::triggered, this, &MainWindow::handleActionStopRecording);
        connect(mActionPlayMacro, &Action::triggered, this, &MainWindow::handleActionPlayMacro);
        connect(mActionUndo, &Action::triggered, this, &MainWindow::handleActionUndo);

        connect(UserActionManager::instance(), &UserActionManager::canUndoLastAction, this, &MainWindow::enableUndo);
        connect(sSettingStyle, &SettingsItemDropdown::intChanged, this, &MainWindow::reloadStylsheet);
        enableUndo(false);

        restoreState();

        //    PluginManagerWidget* widget = new PluginManagerWidget(nullptr);
        //    widget->setPluginModel(mPluginModel);
        //    widget->show();
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

    QString MainWindow::openIconPath() const
    {
        return mOpenIconPath;
    }

    QString MainWindow::openIconStyle() const
    {
        return mOpenIconStyle;
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

    QString MainWindow::settingsIconPath() const
    {
        return mSettingsIconPath;
    }

    QString MainWindow::settingsIconStyle() const
    {
        return mSettingsIconStyle;
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

    void MainWindow::setOpenIconPath(const QString& path)
    {
        mOpenIconPath = path;
    }

    void MainWindow::setOpenIconStyle(const QString& style)
    {
        mOpenIconStyle = style;
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

    void MainWindow::setSettingsIconPath(const QString& path)
    {
        mSettingsIconPath = path;
    }

    void MainWindow::setSettingsIconStyle(const QString& style)
    {
        mSettingsIconStyle = style;
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

    void MainWindow::toggleSettings()
    {
        if (mStackedWidget->currentWidget() == mSettings)
        {
            closeSettings();
        }
        else
        {
            mSettings->activate();
            mStackedWidget->setCurrentWidget(mSettings);
        }
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

    void MainWindow::setPluginParameter()
    {
        QAction* act = static_cast<QAction*>(sender());
        if (!act) return;
        BasePluginInterface* bpif = static_cast<BasePluginInterface*>(act->data().value<void*>());
        if (!bpif) return;
        PluginParameterDialog ppd(bpif,this);
        ppd.exec();
    }

    void MainWindow::pluginMenu()
    {
        QMap<QString,void*> plugins[2];   // 0 = configurable  1 = only listed
        for (const std::string& pluginName : plugin_manager::get_plugin_names())
        {
            BasePluginInterface* bpif = plugin_manager::get_plugin_instance(pluginName);
            if (!bpif) continue;
            plugins[bpif->get_parameter().empty()?1:0].insert(QString::fromStdString(pluginName),bpif);
        }

        for (auto it = plugins[0].constBegin(); it != plugins[0].constEnd(); ++it)
        {
            QAction* act = mMenuPlugin->addAction(it.key());
            act->setData(QVariant::fromValue<void*>(it.value()));
            connect(act,&QAction::triggered,this,&MainWindow::setPluginParameter);
        }
        mMenuPlugin->addSeparator();
        for (auto it = plugins[1].constBegin(); it != plugins[1].constEnd(); ++it)
        {
            QAction* act = mMenuPlugin->addAction(it.key());
            act->setDisabled(true);
        }
    }

    void MainWindow::handleActionNew()
    {
        if (gNetlist != nullptr)
        {
            QMessageBox msgBox;
            msgBox.setText("Error");
            msgBox.setInformativeText("You are already working on a file. Restart HAL to switch to a different file.");
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
            msgBox.setText("Error");
            msgBox.setInformativeText("You are already working on a file. Restart HAL to switch to a different file.");
            msgBox.setStyleSheet("QLabel{min-width: 600px;}");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
            return;
        }

        ProjectDirDialog pdd("Open netlist", this);
        if (pdd.exec() != QDialog::Accepted) return;
        if (pdd.selectedFiles().isEmpty()) return;
        ActionOpenNetlistFile* act = new ActionOpenNetlistFile(ActionOpenNetlistFile::OpenProject, pdd.selectedFiles().at(0));
        act->exec();
    }

    void MainWindow::handleActionImport()
    {
        if (gNetlist != nullptr)
        {
            QMessageBox msgBox;
            msgBox.setText("Error");
            msgBox.setInformativeText("You are already working on a file. Restart HAL to switch to a different file.");
            msgBox.setStyleSheet("QLabel{min-width: 600px;}");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
            return;
        }

        QString title = "Import Netlist";
        QString text  = "All Files(*.vhd *.vhdl *.v *.hal);;VHDL Files (*.vhd *.vhdl);;Verilog Files (*.v);;HAL Progress Files (*.hal)";

        // Non native dialogs does not work on macOS. Therefore do net set DontUseNativeDialog!
        QString path = QDir::currentPath();

        if (gGuiState->contains("FileDialog/Path/MainWindow"))
            path = gGuiState->value("FileDialog/Path/MainWindow").toString();

        QString fileName = QFileDialog::getOpenFileName(nullptr, title, path, text, nullptr);

        if (!fileName.isNull())
        {
            gGuiState->setValue("FileDialog/Path/MainWindow", fileName);

            ActionOpenNetlistFile* actOpenfile = new ActionOpenNetlistFile(ActionOpenNetlistFile::ImportFile, fileName);
            actOpenfile->exec();
        }
    }

    void MainWindow::handleProjectOpened(const QString& projDir, const QString& fileName)
    {
        Q_UNUSED(projDir);
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

        mActionGateLibraryManager->setVisible(false);
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

            if (QFileInfo(saveProjectDir).exists())
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
        mActionStartRecording->setEnabled(true);
        mActionStopRecording->setEnabled(false);
        mActionPlayMacro->setEnabled(true);
        UserActionManager* uam = UserActionManager::instance();
        QString macroFile;
        if (uam->hasRecorded())
        {
            macroFile = QFileDialog::getSaveFileName(this, "Save macro to file", ".");
            if (!macroFile.isEmpty() && !macroFile.contains(QChar('.')))
                macroFile += ".xml";
        }
        uam->setStopRecording(macroFile);
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

        gPythonContext->abortThreadAndWait();

        gGraphContextManager->clear();

        clear();

        gContentManager->deleteContent();
        // PYTHON ???
        gSelectionRelay->clear();
        FileManager::get_instance()->closeFile();
        setWindowTitle("HAL");

        mStackedWidget->setCurrentWidget(mWelcomeScreen);

        gNetlistRelay->reset();

        mActionGateLibraryManager->setVisible(true);

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
