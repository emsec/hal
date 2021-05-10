#include "gui/main_window/main_window.h"

#include "gui/action/action.h"
#include "gui/content_manager/content_manager.h"
#include "gui/docking_system/dock_bar.h"
#include "gui/export/export_registered_format.h"
#include "gui/file_manager/file_manager.h"
#include "gui/graphics_effects/overlay_effect.h"
#include "gui/gui_def.h"
#include "gui/gui_globals.h"
#include "gui/logger/logger_widget.h"
#include "gui/main_window/about_dialog.h"
#include "gui/plugin_access_manager/plugin_access_manager.h"
#include "gui/plugin_manager/plugin_manager_widget.h"
#include "gui/plugin_manager/plugin_model.h"
#include "gui/python/python_editor.h"
#include "gui/settings/settings_items/settings_item_dropdown.h"
#include "gui/settings/settings_items/settings_item_keybind.h"
#include "gui/settings/settings_manager.h"
#include "gui/user_action/action_open_netlist_file.h"
#include "gui/welcome_screen/welcome_screen.h"
#include "hal_core/defines.h"
#include "hal_core/netlist/event_system/event_controls.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "hal_core/utilities/log.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFuture>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QtConcurrent>

namespace hal
{
    SettingsItemDropdown* MainWindow::sSettingStyle = nullptr;

    MainWindow::MainWindow(QWidget* parent) : QWidget(parent)
    {
        ensurePolished();    // ADD REPOLISH METHOD
        connect(FileManager::get_instance(), &FileManager::fileOpened, this, &MainWindow::handleFileOpened);

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

        mActionNew          = new Action(this);
        mActionOpen         = new Action(this);
        mActionSave         = new Action(this);
        mActionSaveAs       = new Action(this);
        mActionAbout        = new Action(this);

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
        mActionOpen->setIcon(gui_utility::getStyledSvgIcon(mOpenIconStyle, mOpenIconPath));
        mActionSave->setIcon(gui_utility::getStyledSvgIcon(mSaveIconStyle, mSaveIconPath));
        mActionSaveAs->setIcon(gui_utility::getStyledSvgIcon(mSaveAsIconStyle, mSaveAsIconPath));
        mActionUndo->setIcon(gui_utility::getStyledSvgIcon(mUndoIconStyle, mUndoIconPath));
        mActionSettings->setIcon(gui_utility::getStyledSvgIcon(mSettingsIconStyle, mSettingsIconPath));

        mMenuFile  = new QMenu(mMenuBar);
        mMenuEdit  = new QMenu(mMenuBar);
        mMenuMacro = new QMenu(mMenuBar);
        mMenuHelp  = new QMenu(mMenuBar);

        mMenuBar->addAction(mMenuFile->menuAction());
        mMenuBar->addAction(mMenuEdit->menuAction());
        mMenuBar->addAction(mMenuMacro->menuAction());
        mMenuBar->addAction(mMenuHelp->menuAction());
        mMenuFile->addAction(mActionNew);
        mMenuFile->addAction(mActionOpen);
        mMenuFile->addAction(mActionClose);
        mMenuFile->addAction(mActionSave);
        mMenuFile->addAction(mActionSaveAs);

        QMenu* menuExport = nullptr;
        for (std::string extension : netlist_writer_manager::registered_extensions())
        {
            QString ext = QString::fromStdString(extension);
            if (!menuExport) menuExport = new QMenu("Export …", this);
            Action* action = new Action("Export as " + ext.toUpper(), this);
            action->setData(ext);
            connect(action, &QAction::triggered, this, &MainWindow::handleActionExport);
            menuExport->addAction(action);
        }
        if (menuExport) mMenuFile->addMenu(menuExport);

        mMenuEdit->addAction(mActionUndo);
        mMenuEdit->addSeparator();
        mMenuEdit->addAction(mActionSettings);
        mMenuMacro->addAction(mActionStartRecording);
        mMenuMacro->addAction(mActionStopRecording);
        mMenuMacro->addSeparator();
        mMenuMacro->addAction(mActionPlayMacro);
        mMenuHelp->addAction(mActionAbout);
        mLeftToolBar->addAction(mActionNew);
        mLeftToolBar->addAction(mActionOpen);
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

        setWindowTitle("HAL");
        mActionNew->setText("New Netlist");
        mActionOpen->setText("Open");
        mActionSave->setText("Save");
        mActionSaveAs->setText("Save As");
        mActionUndo->setText("Undo");
        mActionAbout->setText("About");
        mActionSettings->setText("Settings");
        mActionClose->setText("Close Document");
        mMenuFile->setTitle("File");
        mMenuEdit->setTitle("Edit");
        mMenuMacro->setTitle("Macro");
        mMenuHelp->setTitle("Help");

        mPluginModel = new PluginModel(this);

        gPythonContext = std::make_unique<PythonContext>();

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
        connect(shortCutOpenFile, &QShortcut::activated, mActionOpen, &QAction::trigger);
        connect(shortCutSaveFile, &QShortcut::activated, mActionSave, &QAction::trigger);
        connect(shortCutUndoLast, &QShortcut::activated, mActionUndo, &QAction::trigger);

        connect(mActionNew, &Action::triggered, this, &MainWindow::handleActionNew);
        connect(mActionOpen, &Action::triggered, this, &MainWindow::handleActionOpen);
        connect(mActionAbout, &Action::triggered, this, &MainWindow::handleActionAbout);
        //        connect(mActionSchedule, &Action::triggered, this, &MainWindow::toggleSchedule);
        connect(mActionSettings, &Action::triggered, this, &MainWindow::toggleSettings);
        connect(mSettings, &MainSettingsWidget::close, this, &MainWindow::closeSettings);
        connect(mActionSave, &Action::triggered, this, &MainWindow::handleSaveTriggered);
        connect(mActionSaveAs, &Action::triggered, this, &MainWindow::handleSaveAsTriggered);
        //debug
        connect(mActionClose, &Action::triggered, this, &MainWindow::handleActionCloseFile);

        connect(mActionStartRecording, &Action::triggered, this, &MainWindow::handleActionStartRecording);
        connect(mActionStopRecording, &Action::triggered, this, &MainWindow::handleActionStopRecording);
        connect(mActionPlayMacro, &Action::triggered, this, &MainWindow::handleActionPlayMacro);
        connect(mActionUndo,
                &Action::triggered,
                this,
                &MainWindow::handleActionUndo);    //        connect(mActionRunSchedule, &Action::triggered, PluginScheduleManager::get_instance(), &PluginScheduleManager::runSchedule);

        connect(this, &MainWindow::saveTriggered, gContentManager, &ContentManager::handleSaveTriggered);
        connect(this, &MainWindow::saveTriggered, gGraphContextManager, &GraphContextManager::handleSaveTriggered);

        connect(UserActionManager::instance(), &UserActionManager::canUndoLastAction, this, &MainWindow::enableUndo);
        connect(sSettingStyle, &SettingsItemDropdown::intChanged, this, &MainWindow::reloadStylsheet);
        enableUndo(false);

        restoreState();

        //    PluginManagerWidget* widget = new PluginManagerWidget(nullptr);
        //    widget->setPluginModel(mPluginModel);
        //    widget->show();

        //setGraphicsEffect(new OverlayEffect());

        //ReminderOverlay* o = new ReminderOverlay(this);
        //Q_UNUSED(o)
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

    extern void runMain(const QString fileName, const QList<QString> plugins);

    void MainWindow::runPluginTriggered(const QString& name)
    {
        if (!FileManager::get_instance()->fileOpen())
        {
            return;
        }

        //    QString document = m_document_manager->get_input_file_name();
        //    QList<QString> plugins;
        //    plugins.append(name);
        //    QFuture<void> future = QtConcurrent::run(runMain, document, plugins);

        auto args            = plugin_access_manager::requestArguments(name.toStdString());
        QFuture<void> future = QtConcurrent::run(plugin_access_manager::runPlugin, name.toStdString(), &args);
    }

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

        QString title = "Create New Netlist";
        QString text  = "Please select a gate library";

        QStringList items;
        auto libraries = gate_library_manager::get_gate_libraries();
        for (const auto& lib : libraries)
        {
            items.append(QString::fromStdString(lib->get_name()));
        }
        bool ok          = false;
        QString selected = QInputDialog::getItem(this, title, text, items, 0, false, &ok);

        if (ok)
        {
            // DEBUG -- REMOVE WHEN GUI CAN HANDLE EVENTS DURING CREATION
            event_controls::enable_all(false);
            auto selected_lib = libraries[items.indexOf(selected)];
            gNetlistOwner     = netlist_factory::create_netlist(selected_lib);
            gNetlist          = gNetlistOwner.get();
            // DEBUG -- REMOVE WHEN GUI CAN HANDLE EVENTS DURING CREATION
            event_controls::enable_all(true);
            Q_EMIT FileManager::get_instance()->fileOpened("new netlist");
        }
    }

    void MainWindow::handleActionOpen()
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

        QString title = "Open File";
        QString text  = "All Files(*.vhd *.vhdl *.v *.hal);;VHDL Files (*.vhd *.vhdl);;Verilog Files (*.v);;HAL Progress Files (*.hal)";

        // Non native dialogs does not work on macOS. Therefore do net set DontUseNativeDialog!
        QString path = QDir::currentPath();

        if (gGuiState->contains("FileDialog/Path/MainWindow"))
            path = gGuiState->value("FileDialog/Path/MainWindow").toString();

        QString fileName = QFileDialog::getOpenFileName(nullptr, title, path, text, nullptr);

        if (!fileName.isNull())
        {
            gGuiState->setValue("FileDialog/Path/MainWindow", fileName);

            // DEBUG -- REMOVE WHEN GUI CAN HANDLE EVENTS DURING CREATION
            event_controls::enable_all(false);
            ActionOpenNetlistFile* actOpenfile = new ActionOpenNetlistFile(fileName);
            actOpenfile->exec();
            // DEBUG -- REMOVE WHEN GUI CAN HANDLE EVENTS DURING CREATION
            event_controls::enable_all(true);
        }
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
    }

    void MainWindow::handleActionExport()
    {
        if (!gNetlist) return;
        QAction* act = static_cast<QAction*>(sender());
        if (!act || act->data().isNull()) return;

        qDebug() << "xport to" << act->data().toString();
        ExportRegisteredFormat erf(act->data().toString());
        if (erf.queryFilename())
            erf.exportNetlist();

    }

    void MainWindow::handleSaveAsTriggered()
    {
        QString filename = saveHandler();
        if (!filename.isEmpty())
            gContentManager->setWindowTitle(filename);
    }

    void MainWindow::handleSaveTriggered()
    {
        saveHandler(FileManager::get_instance()->fileName());
    }

    QString MainWindow::saveHandler(const QString &filename)
    {
        if (!gNetlist) return QString();

        std::filesystem::path path;
        QString newName;

        if (filename.isEmpty())
        {
            QString title = "Save File";
            QString text  = "HAL Progress Files (*.hal)";

            // Non native dialogs does not work on macOS. Therefore do net set DontUseNativeDialog!
            newName = QFileDialog::getSaveFileName(nullptr, title, QDir::currentPath(), text, nullptr);
            if (!newName.isNull())
            {
                path = newName.toStdString();
            }
            else
            {
                return QString();
            }
        }
        else
        {
            path = filename.toStdString();
        }


        path.replace_extension(".hal");
        netlist_serializer::serialize_to_file(gNetlist, path);

        gFileStatusManager->netlistSaved();
        FileManager::get_instance()->watchFile(QString::fromStdString(path.string()));

        Q_EMIT saveTriggered();

        return newName;
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

        gGraphContextManager->clear();

        clear();

        gContentManager->deleteContent();
        // PYTHON ???
        gSelectionRelay->clear();
        FileManager::get_instance()->closeFile();
        setWindowTitle("HAL");

        mStackedWidget->setCurrentWidget(mWelcomeScreen);

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
