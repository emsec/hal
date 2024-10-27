#include "gui/main_window/file_actions.h"
#include "gui/main_window/main_window.h"
#include "gui/gui_utils/graphics.h"
#include "gui/settings/settings_items/settings_item_keybind.h"
#include "gui/gatelibrary_management/gatelibrary_content_widget.h"
#include "gui/file_status_manager/file_status_manager.h"
#include "gui/gui_globals.h"

#include <QShortcut>

namespace hal {

    FileActions::FileActions(QWidget *parent)
        : QWidget(parent), mGatelibReference(nullptr), mGuiPluginReference(nullptr)
    {
        repolish();
        mMainWindowReference = dynamic_cast<MainWindow*>(parent);
        mActionCreate             = new Action(this);
        mActionOpen               = new Action(this);
        mActionClose              = new Action(this);
        mActionSave               = new Action(this);
        mActionSaveAs             = new Action(this);

        mActionCreate->setIcon(gui_utility::getStyledSvgIcon(mNewFileIconStyle, mNewFileIconPath, mDisabledIconStyle));
        mActionOpen->setIcon(gui_utility::getStyledSvgIcon(mOpenProjIconStyle, mOpenProjIconPath, mDisabledIconStyle));
        mActionClose->setIcon(gui_utility::getStyledSvgIcon(mCloseIconStyle, mCloseIconPath));
        mActionSave->setIcon(gui_utility::getStyledSvgIcon(mEnabledIconStyle, mSaveIconPath, mDisabledIconStyle));
        mActionSaveAs->setIcon(gui_utility::getStyledSvgIcon(mEnabledIconStyle, mSaveAsIconPath, mDisabledIconStyle));

        mSettingSaveFile =
            new SettingsItemKeybind("HAL Shortcut 'Save File'", "keybinds/project_save_file", QKeySequence("Ctrl+S"), "Keybindings:Global", "Keybind for saving the currently opened file.");
        mSettingCreateFile = new SettingsItemKeybind(
            "HAL Shortcut 'Create Empty Netlist'", "keybinds/project_create_file", QKeySequence("Ctrl+N"), "Keybindings:Global", "Keybind for creating a new and empty netlist in HAL.");

        mSettingOpenFile = new SettingsItemKeybind("HAL Shortcut 'Open File'", "keybinds/project_open_file", QKeySequence("Ctrl+O"), "Keybindings:Global", "Keybind for opening a new File in HAL.");

        QShortcut* shortCutNewFile  = new QShortcut(mSettingCreateFile->value().toString(), this);
        QShortcut* shortCutOpenFile = new QShortcut(mSettingOpenFile->value().toString(),   this);
        QShortcut* shortCutSaveFile = new QShortcut(mSettingSaveFile->value().toString(),   this);

        connect(mSettingCreateFile, &SettingsItemKeybind::keySequenceChanged, shortCutNewFile,  &QShortcut::setKey);
        connect(mSettingOpenFile,   &SettingsItemKeybind::keySequenceChanged, shortCutOpenFile, &QShortcut::setKey);
        connect(mSettingSaveFile,   &SettingsItemKeybind::keySequenceChanged, shortCutSaveFile, &QShortcut::setKey);

        connect(shortCutNewFile,  &QShortcut::activated, mActionCreate, &QAction::trigger);
        connect(shortCutOpenFile, &QShortcut::activated, mActionOpen,   &QAction::trigger);
        connect(shortCutSaveFile, &QShortcut::activated, mActionSave,   &QAction::trigger);

        connect(gFileStatusManager, &FileStatusManager::status_changed, this, &FileActions::handleFileStatusChanged);
        setup();
        hide();
    }

    void FileActions::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);
    }

    void FileActions::handleFileStatusChanged(bool gateLibrary, bool isDirty)
    {
        if (gateLibrary == (mGatelibReference==nullptr)) return;

        mActionSave->setEnabled(isDirty);
        mActionSaveAs->setEnabled(isDirty);
    }

    void FileActions::setup(GateLibraryManager *glcw, GuiPluginManager *plmgr)
    {
        mGatelibReference = glcw;
        mGuiPluginReference = plmgr;
        mActionCreate->disconnect();
        mActionOpen->disconnect();
        mActionClose->disconnect();
        mActionSave->disconnect();
        mActionSaveAs->disconnect();
        if (mGatelibReference)
        {
            mActionCreate->setText("New Gate Library");
            mActionOpen->setText("Open Gate Library");
            mActionClose->setText("Close Gate Lib Manager");
            mActionSave->setText("Save Gate Library");
            mActionSaveAs->setText("Save Gate Library As");
            connect(mActionCreate, &Action::triggered, mGatelibReference, &GateLibraryManager::handleCreateAction);
            connect(mActionOpen,   &Action::triggered, mMainWindowReference, &MainWindow::handleActionGatelibraryManager);
            connect(mActionClose, &Action::triggered, mGatelibReference, &GateLibraryManager::handleCancelClicked);
            connect(mActionSave, &Action::triggered, mGatelibReference, &GateLibraryManager::handleSaveAction);
            connect(mActionSaveAs, &Action::triggered, mGatelibReference, &GateLibraryManager::handleSaveAsAction);
            mActionCreate->setEnabled(true);
            mActionSave->setEnabled(gFileStatusManager->isGatelibModified());
            mActionSaveAs->setEnabled(gFileStatusManager->isGatelibModified());
        }
        else if (mGuiPluginReference)
        {
            mActionCreate->setText("");
            mActionOpen->setText("Load HAL Plugin");
            mActionClose->setText("Close Plugin Manager");
            mActionSave->setText("");
            mActionSaveAs->setText("");
//            connect(mActionOpen,  &Action::triggered, mMainWindowReference, &MainWindow::handleActionGatelibraryManager);
//            connect(mActionClose, &Action::triggered, mGatelibReference, &GateLibraryManager::handleCancelClicked);
            mActionCreate->setDisabled(true);
            mActionSave->setDisabled(true);
            mActionSaveAs->setDisabled(true);
        }
        else
        {
            mActionCreate->setText("New Project");
            mActionCreate->setEnabled(true);
            mActionOpen->setText("Open Project");
            mActionClose->setText("Close Project");
            mActionSave->setText("Save HAL Project");
            mActionSaveAs->setText("Save HAL Project As");
            connect(mActionCreate, &Action::triggered, mMainWindowReference, &MainWindow::handleActionNew);
            connect(mActionOpen,   &Action::triggered, mMainWindowReference, &MainWindow::handleActionOpenProject);
            connect(mActionClose, &Action::triggered, mMainWindowReference, &MainWindow::handleActionCloseFile);
            connect(mActionSave,   &Action::triggered, mMainWindowReference, &MainWindow::handleSaveTriggered);
            connect(mActionSaveAs, &Action::triggered, mMainWindowReference, &MainWindow::handleSaveAsTriggered);
            mActionCreate->setEnabled(true);
            mActionSave->setEnabled(gFileStatusManager->modifiedFilesExisting());
            mActionSaveAs->setEnabled(gFileStatusManager->modifiedFilesExisting());
        }
    }
}
