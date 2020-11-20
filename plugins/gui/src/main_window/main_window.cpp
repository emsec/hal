#include "gui/main_window/main_window.h"

#include "gui/action/action.h"
#include "gui/content_manager/content_manager.h"
#include "gui/docking_system/dock_bar.h"
#include "gui/file_manager/file_manager.h"
#include "gui/graphics_effects/overlay_effect.h"
#include "gui/gui_def.h"
#include "gui/gui_globals.h"
#include "gui/logger/logger_widget.h"
#include "gui/main_window/about_dialog.h"
#include "gui/notifications/notification.h"
#include "gui/overlay/reminder_overlay.h"
#include "gui/plugin_access_manager/plugin_access_manager.h"
#include "gui/plugin_management/plugin_schedule_manager.h"
#include "gui/plugin_management/plugin_schedule_widget.h"
#include "gui/plugin_manager/plugin_manager_widget.h"
#include "gui/plugin_manager/plugin_model.h"
#include "gui/python/python_editor.h"
#include "gui/welcome_screen/welcome_screen.h"

#include "hal_core/defines.h"
#include "hal_core/netlist/event_system/event_controls.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/hdl_parser/hdl_parser_manager.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "hal_core/utilities/log.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFuture>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QShortcut>
#include <QtConcurrent>

namespace hal
{
    MainWindow::MainWindow(QWidget* parent) : QWidget(parent), mScheduleWidget(new PluginScheduleWidget())
      // , mActionSchedule(new Action(this))
      // , mActionContent(new Action(this))
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

        mStackedWidget->addWidget(mScheduleWidget);

        mSettings = new MainSettingsWidget();
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
        mActionAbout        = new Action(this);
        //mActionRunSchedule = new Action(this);
        //mActionContent      = new Action(this);
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
//        mActionSchedule->setIcon(gui_utility::getStyledSvgIcon(mScheduleIconStyle, mScheduleIconPath));
//        mActionRunSchedule->setIcon(gui_utility::getStyledSvgIcon(mRunIconStyle, mRunIconPath));
//        mActionContent->setIcon(gui_utility::getStyledSvgIcon(mContentIconStyle, mContentIconPath));
        mActionSettings->setIcon(gui_utility::getStyledSvgIcon(mSettingsIconStyle, mSettingsIconPath));

        mMenuFile = new QMenu(mMenuBar);
        mMenuEdit = new QMenu(mMenuBar);
        mMenuHelp = new QMenu(mMenuBar);

        mMenuBar->addAction(mMenuFile->menuAction());
        mMenuBar->addAction(mMenuEdit->menuAction());
        mMenuBar->addAction(mMenuHelp->menuAction());
        mMenuFile->addAction(mActionNew);
        mMenuFile->addAction(mActionOpen);
        //mMenuFile->addAction(mActionClose);
        mMenuFile->addAction(mActionSave);
        mMenuEdit->addAction(mActionSettings);
        mMenuHelp->addAction(mActionAbout);
        mLeftToolBar->addAction(mActionNew);
        mLeftToolBar->addAction(mActionOpen);
        mLeftToolBar->addAction(mActionSave);
        //    mLeftToolBar->addSeparator();
//        mLeftToolBar->addAction(mActionSchedule);
//        mLeftToolBar->addAction(mActionRunSchedule);
//        mLeftToolBar->addAction(mActionContent);
        //    mLeftToolBar->addSeparator();
        //    mRightToolBar->addSeparator();
        mRightToolBar->addAction(mActionSettings);

        gKeybindManager->bind(mActionNew, "keybinds/project_create_file");
        gKeybindManager->bind(mActionOpen, "keybinds/project_open_file");
        gKeybindManager->bind(mActionSave, "keybinds/project_save_file");
 //       gKeybindManager->bind(mActionRunSchedule, "keybinds/schedule_run");

        setWindowTitle("HAL");
        mActionNew->setText("New Netlist");
        mActionOpen->setText("Open");
        mActionSave->setText("Save");
        mActionAbout->setText("About");
//        mActionSchedule->setText("Edit Schedule");
//        mActionRunSchedule->setText("Run Schedule");
//        mActionContent->setText("Content (Disabled)");
        mActionSettings->setText("Settings");
        mActionClose->setText("Close Document");
        mMenuFile->setTitle("File");
        mMenuEdit->setTitle("Edit");
        mMenuHelp->setTitle("Help");

        mAboutDialog  = new AboutDialog(this);
        mPluginModel = new PluginModel(this);

        gPythonContext = std::make_unique<PythonContext>();

        gContentManager = new ContentManager(this);

        connect(mActionNew, &Action::triggered, this, &MainWindow::handleActionNew);
        connect(mActionOpen, &Action::triggered, this, &MainWindow::handleActionOpen);
        connect(mActionAbout, &Action::triggered, mAboutDialog, &AboutDialog::exec);
//        connect(mActionSchedule, &Action::triggered, this, &MainWindow::toggleSchedule);
        connect(mActionSettings, &Action::triggered, this, &MainWindow::toggleSettings);
        connect(mSettings, &MainSettingsWidget::close, this, &MainWindow::closeSettings);
        connect(mActionSave, &Action::triggered, this, &MainWindow::handleSaveTriggered);
        //debug
        connect(mActionClose, &Action::triggered, this, &MainWindow::handleActionClosed);

//        connect(mActionRunSchedule, &Action::triggered, PluginScheduleManager::get_instance(), &PluginScheduleManager::runSchedule);

        connect(this, &MainWindow::saveTriggered, gContentManager, &ContentManager::handleSaveTriggered);

        restoreState();

        //    PluginManagerWidget* widget = new PluginManagerWidget(nullptr);
        //    widget->setPluginModel(mPluginModel);
        //    widget->show();

        //setGraphicsEffect(new OverlayEffect());

        //ReminderOverlay* o = new ReminderOverlay(this);
        //Q_UNUSED(o)
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

    QString MainWindow::scheduleIconPath() const
    {
        return mScheduleIconPath;
    }

    QString MainWindow::scheduleIconStyle() const
    {
        return mScheduleIconStyle;
    }

    QString MainWindow::runIconPath() const
    {
        return mRunIconPath;
    }

    QString MainWindow::runIconStyle() const
    {
        return mRunIconStyle;
    }

    QString MainWindow::contentIconPath() const
    {
        return mContentIconPath;
    }

    QString MainWindow::contentIconStyle() const
    {
        return mContentIconStyle;
    }

    QString MainWindow::settingsIconPath() const
    {
        return mSettingsIconPath;
    }

    QString MainWindow::settingsIconStyle() const
    {
        return mSettingsIconStyle;
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

    void MainWindow::setScheduleIconPath(const QString& path)
    {
        mScheduleIconPath = path;
    }

    void MainWindow::setScheduleIconStyle(const QString& style)
    {
        mScheduleIconStyle = style;
    }

    void MainWindow::setRunIconPath(const QString& path)
    {
        mRunIconPath = path;
    }

    void MainWindow::setRunIconStyle(const QString& style)
    {
        mRunIconStyle = style;
    }

    void MainWindow::setContentIconPath(const QString& path)
    {
        mContentIconPath = path;
    }

    void MainWindow::setContentIconStyle(const QString& style)
    {
        mContentIconStyle = style;
    }

    void MainWindow::setSettingsIconPath(const QString& path)
    {
        mSettingsIconPath = path;
    }

    void MainWindow::setSettingsIconStyle(const QString& style)
    {
        mSettingsIconStyle = style;
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

    // GENERALIZE TOGGLE METHODS
    void MainWindow::toggleSchedule()
    {
        if (mStackedWidget->currentWidget() == mScheduleWidget)
        {
            if (FileManager::get_instance()->fileOpen())
                mStackedWidget->setCurrentWidget(mLayoutArea);
            else
                mStackedWidget->setCurrentWidget(mWelcomeScreen);
        }
        else
            mStackedWidget->setCurrentWidget(mScheduleWidget);
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
            gNetlistOwner   = netlist_factory::create_netlist(selected_lib);
            gNetlist         = gNetlistOwner.get();
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
        QString fileName = QFileDialog::getOpenFileName(nullptr, title, QDir::currentPath(), text, nullptr);

        if (!fileName.isNull())
        {
            // DEBUG -- REMOVE WHEN GUI CAN HANDLE EVENTS DURING CREATION
            event_controls::enable_all(false);
            FileManager::get_instance()->openFile(fileName);
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

    void MainWindow::handleSaveTriggered()
    {
        if (gNetlist)
        {
            std::filesystem::path path = FileManager::get_instance()->fileName().toStdString();

            if (path.empty())
            {
                QString title = "Save File";
                QString text  = "HAL Progress Files (*.hal)";

                // Non native dialogs does not work on macOS. Therefore do net set DontUseNativeDialog!
                QString fileName = QFileDialog::getSaveFileName(nullptr, title, QDir::currentPath(), text, nullptr);
                if (!fileName.isNull())
                {
                    path = fileName.toStdString();
                }
                else
                {
                    return;
                }
            }

            path.replace_extension(".hal");
            netlist_serializer::serialize_to_file(gNetlist, path);

            gFileStatusManager->flushUnsavedChanges();
            FileManager::get_instance()->watchFile(QString::fromStdString(path.string()));

            Q_EMIT saveTriggered();
        }
    }

    void MainWindow::handleActionClosed()
    {
    }

    void MainWindow::onActionQuitTriggered()
    {
        close();
    }

    void MainWindow::closeEvent(QCloseEvent* event)
    {
        //check for unsaved changes and show confirmation dialog
        if (gFileStatusManager->modifiedFilesExisting())
        {
            QMessageBox msgBox;
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
            {
                event->ignore();
                return;
            }
        }

        FileManager::get_instance()->closeFile();

        saveState();
        event->accept();
        // hack, remove later
        gContentManager->hackDeleteContent();
        qApp->quit();
    }

    void MainWindow::restoreState()
    {
        QPoint pos = gSettingsManager->get("MainWindow/position", QPoint(0, 0)).toPoint();
        move(pos);
        QRect rect = QApplication::desktop()->screenGeometry();
        QSize size = gSettingsManager->get("MainWindow/size", QSize(rect.width(), rect.height())).toSize();
        resize(size);
        //restore state of all subwindows
        mLayoutArea->initSplitterSize(size);
    }

    void MainWindow::saveState()
    {
        gSettingsManager->update("MainWindow/position", pos());
        gSettingsManager->update("MainWindow/size", size());
        //save state of all subwindows and everything else that might need to be restored on the next program start
        gSettingsManager->sync();
    }

    void MainWindow::addContent(ContentWidget* widget, int index, content_anchor anchor)
    {
        mLayoutArea->addContent(widget, index, anchor);
    }
}    // namespace hal
