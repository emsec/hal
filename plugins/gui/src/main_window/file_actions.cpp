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
        : QWidget(parent), mGatelibReference(nullptr)
    {
        repolish();
        mMainWindowReference = dynamic_cast<MainWindow*>(parent);
        mActionSave               = new Action(this);
        mActionSaveAs             = new Action(this);

        mActionSave->setIcon(gui_utility::getStyledSvgIcon(mEnabledIconStyle, mSaveIconPath, mDisabledIconStyle));
        mActionSaveAs->setIcon(gui_utility::getStyledSvgIcon(mEnabledIconStyle, mSaveAsIconPath, mDisabledIconStyle));

        mSettingSaveFile =
            new SettingsItemKeybind("HAL Shortcut 'Save File'", "keybinds/project_save_file", QKeySequence("Ctrl+S"), "Keybindings:Global", "Keybind for saving the currently opened file.");
        QShortcut* shortCutSaveFile = new QShortcut(mSettingSaveFile->value().toString(), this);
        connect(mSettingSaveFile, &SettingsItemKeybind::keySequenceChanged, shortCutSaveFile, &QShortcut::setKey);
        connect(shortCutSaveFile, &QShortcut::activated, mActionSave, &QAction::trigger);
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

    void FileActions::setup(GateLibraryManager *glcw)
    {
        mGatelibReference = glcw;
        mActionSave->disconnect();
        mActionSaveAs->disconnect();
        if (mGatelibReference)
        {
            mActionSave->setText("Save Gate Libraray");
            mActionSaveAs->setText("Save Gate Library As");
            connect(mActionSave, &Action::triggered, mGatelibReference, &GateLibraryManager::handleSaveAction);
            connect(mActionSaveAs, &Action::triggered, mGatelibReference, &GateLibraryManager::handleSaveAsAction);
            mActionSave->setEnabled(gFileStatusManager->isGatelibModified());
            mActionSaveAs->setEnabled(gFileStatusManager->isGatelibModified());
        }
        else
        {
            mActionSave->setText("Save HAL Project");
            mActionSaveAs->setText("Save HAL Project As");
            connect(mActionSave, &Action::triggered, mMainWindowReference, &MainWindow::handleSaveTriggered);
            connect(mActionSaveAs, &Action::triggered, mMainWindowReference, &MainWindow::handleSaveAsTriggered);
            mActionSave->setEnabled(gFileStatusManager->modifiedFilesExisting());
            mActionSaveAs->setEnabled(gFileStatusManager->modifiedFilesExisting());
        }
    }
}
