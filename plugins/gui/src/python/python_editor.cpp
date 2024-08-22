#include "gui/python/python_editor.h"

#include "gui/action/action.h"
#include "gui/code_editor/syntax_highlighter/python_syntax_highlighter.h"
#include "gui/file_manager/file_manager.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"
#include "gui/python/python_code_editor.h"
#include "gui/python/python_editor_code_completion_dialog.h"
#include "gui/searchbar/searchbar.h"
#include "gui/settings/settings_items/settings_item_checkbox.h"
#include "gui/settings/settings_items/settings_item_keybind.h"
#include "gui/settings/settings_items/settings_item_spinbox.h"
#include "gui/splitter/splitter.h"
#include "gui/toolbar/toolbar.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

#include <QAction>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QShortcut>
#include <QTabBar>
#include <QTextDocumentFragment>
#include <QTextStream>
#include <QToolButton>
#include <QVBoxLayout>
#include <chrono>
#include <fstream>
#include "hal_core/netlist/project_manager.h"
#include "hal_core/utilities/project_directory.h"
#include <QDir>
#include "rapidjson/filereadstream.h"

namespace hal
{
    PythonSerializer::PythonSerializer()
        : ProjectSerializer("pythoneditor")
    {
        ProjectManager* pm = ProjectManager::instance();
        if (pm->get_project_status() == ProjectManager::ProjectStatus::NONE) return;

        mSaveDir = QString::fromStdString(pm->get_project_directory().get_filename("py").string());
        QDir().mkpath(mSaveDir);

        std::string relname = pm->get_filename(m_name);
        if (!relname.empty())
            restoreTabs(pm->get_project_directory(), relname);
    }

    QString PythonSerializer::sPythonRelDir = "py";
    std::string PythonSerializer::sControlFileName = "pythoneditor.json";

    std::string PythonSerializer::serialize(Netlist* netlist, const std::filesystem::path &savedir, bool isAutosave)
    {
        Q_UNUSED(netlist);
        QDir pyDir(QString::fromStdString((savedir / sPythonRelDir.toStdString()).string()));
        gContentManager->getPythonEditorWidget()->saveAllTabs(pyDir.absolutePath(),isAutosave);

        return serialize_control(savedir,isAutosave);
    }

    bool PythonSerializer::write_control_file(const std::filesystem::path& savedir, const std::vector<PythonEditorControlEntry>& tabinfo)
    {
        QDir workDir(QString::fromStdString(savedir.empty()
                                            ? ProjectManager::instance()->get_project_directory().get_canonical_path().string()
                                            : savedir.string()));
        QDir pyDir(workDir.absoluteFilePath(sPythonRelDir));
        QString PythonEditorControlFile = workDir.absoluteFilePath(QString::fromStdString(sControlFileName));

        JsonWriteDocument doc;

        doc["python_dir"] = sPythonRelDir.toStdString();
        JsonWriteArray& tabArr = doc.add_array("tabs");

        for (const PythonEditorControlEntry& pece : tabinfo)
        {
            JsonWriteObject& tabObj = tabArr.add_object();
            tabObj["tab"]  = pece.tabInx;
            if (pece.active)
                tabObj["active"] = true;
            if (!pece.restore.empty())
                tabObj["restore"] = pece.restore;
            tabObj["filename"] = pece.filename;
            tabObj.close();
        }

        tabArr.close();
        return doc.serialize(PythonEditorControlFile.toStdString());
    }

    std::string PythonSerializer::serialize_control(const std::filesystem::path& savedir, bool isAutosave)
    {
        QDir workDir(QString::fromStdString(savedir.empty()
                                            ? ProjectManager::instance()->get_project_directory().get_canonical_path().string()
                                            : savedir.string()));
        QDir pyDir(workDir.absoluteFilePath(sPythonRelDir));

        std::vector<PythonEditorControlEntry> tabinfo;

        PythonEditor* pedit = gContentManager->getPythonEditorWidget();
        if (!pedit) return std::string();

        QTabWidget* tabw = pedit->getTabWidget();
        for (int tabInx=0; tabInx < tabw->count(); tabInx++)
        {
            PythonCodeEditor* pce = pedit->getPythonEditor(tabInx);
            if (!pce) continue;

            PythonEditorControlEntry pece;
            pece.tabInx = tabInx;
            pece.active = (tabw->currentWidget() == pce);

            QString tabPath = pce->getRelFilename();
            if (tabPath.isEmpty())
                tabPath = pyDir.absoluteFilePath(pedit->unnamedFilename(tabInx));
            else if (isAutosave)
            {
                pece.restore = tabPath.toStdString();
                tabPath = pyDir.absoluteFilePath(pedit->autosaveFilename(tabInx));
            }
            QString pydirPrefix = pyDir.absolutePath() + "/";
            QString tabFilename = tabPath.startsWith(pydirPrefix)
                    ? tabPath.mid(pydirPrefix.size())
                    : tabPath;
            pece.filename = tabFilename.toStdString();
            tabinfo.push_back(pece);
        }

        if (!write_control_file(savedir,tabinfo))
        {
            log_warning("gui", "Failed to save python editor control file to project dir {}.", workDir.absolutePath().toStdString());
        }
        return sControlFileName;
    }

    void PythonSerializer::deserialize(Netlist* netlist, const std::filesystem::path& loaddir)
    {
        Q_UNUSED(netlist);
        std::string relname = ProjectManager::instance()->get_filename(m_name);
        if (!relname.empty())
            restoreTabs(loaddir, relname);
    }

    void PythonSerializer::restoreTabs(const std::filesystem::path& loaddir, const std::string& jsonfile)
    {
        std::filesystem::path jsonpath(loaddir);
        jsonpath.append(jsonfile);

        FILE* pytabFile = fopen(jsonpath.string().c_str(), "rb");
        if (pytabFile == NULL)
        {
            log_error("GroupingSerializer::deserialize", "unable to open '{}'.", jsonpath.string());
            return;
        }

        PythonEditor* pedit = gContentManager->getPythonEditorWidget();

        char buffer[65536];
        rapidjson::FileReadStream frs(pytabFile, buffer, sizeof(buffer));
        rapidjson::Document document;
        document.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(frs);

        QDir pyDir(QString::fromStdString((loaddir / std::filesystem::path(document.HasMember("python_dir")?document["python_dir"].GetString():std::string("py"))).string()));

        bool restoreAutosave = false;

        int activeInx = -1;
        if (document.HasMember("tabs"))
        {
            for (const rapidjson::Value& tabVal : document["tabs"].GetArray())
            {
                if (!tabVal.HasMember("tab") || !tabVal.HasMember("filename")) continue;
                int tabInx      = tabVal["tab"].GetUint();
                QString tabFilename = QString::fromStdString(tabVal["filename"].GetString());
                QString tabPath = QFileInfo(tabFilename).isRelative() ? pyDir.absoluteFilePath(tabFilename) : tabFilename;
                if (tabFilename.startsWith(".autosave_tab") && tabVal.HasMember("restore"))
                {
                    QString restorePath = QString::fromStdString(tabVal["restore"].GetString());
                    if (QFileInfo(restorePath).exists()) QFile::remove(restorePath);
                    QFile::copy(tabPath,restorePath);
                    tabPath = restorePath;
                    restoreAutosave = true;
                }
                pedit->tabLoadFile(tabInx, tabPath);
                if (tabVal.HasMember("active"))
                    activeInx = tabInx;
            }
        }

        if (activeInx >= 0)
            pedit->getTabWidget()->setCurrentIndex(activeInx);

        if (restoreAutosave)
        {
            std::filesystem::path workdir = ProjectManager::instance()->get_project_directory().get_canonical_path();
            pedit->saveAllTabs(QString::fromStdString(workdir.string()),false);
            pedit->saveControl();
        }
    }


    PythonEditor::PythonEditor(QWidget* parent)
        : ContentWidget("Python Editor", parent), PythonContextSubscriber(), mSearchbar(new Searchbar(this)), mActionOpenFile(new Action(this)), mActionRun(new Action(this)),
          mActionSave(new Action(this)), mActionSaveAs(new Action(this)), mActionToggleMinimap(new Action(this)), mActionNewFile(new Action(this)),
          mFileWatcher(nullptr)
    {
        ensurePolished();
        mNewFileCounter = 0;
        mLastClickTime  = 0;

        mTabWidget = new QTabWidget(this);
        mTabWidget->setTabsClosable(true);
        mTabWidget->setMovable(true);
        // we need to grab mouse events from the tab bar
        mTabWidget->tabBar()->installEventFilter(this);
        mContentLayout->addWidget(mTabWidget);
        connect(mTabWidget, &QTabWidget::tabCloseRequested, this, &PythonEditor::handleTabCloseRequested);
        mContentLayout->addWidget(mSearchbar);
        mSearchbar->hide();
        mSearchbar->setEmitTextWithFlags(false);

        mActionOpenFile->setIcon(gui_utility::getStyledSvgIcon(mOpenIconStyle, mOpenIconPath));
        mActionSave->setIcon(gui_utility::getStyledSvgIcon(mSaveIconStyle, mSaveIconPath));
        mActionSaveAs->setIcon(gui_utility::getStyledSvgIcon(mSaveAsIconStyle, mSaveAsIconPath));
        mActionRun->setIcon(gui_utility::getStyledSvgIcon(mRunIconStyle, mRunIconPath));
        mActionToggleMinimap->setIcon(gui_utility::getStyledSvgIcon(mToggleMinimapIconStyle, mToggleMinimapIconPath));
        mActionNewFile->setIcon(gui_utility::getStyledSvgIcon(mNewFileIconStyle, mNewFileIconPath));
        mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchIconStyle, mSearchIconPath));

        mActionOpenFile->setText("Open Script");
        mActionSave->setText("Save");
        mActionSaveAs->setText("Save as");
        mActionRun->setText("Execute Script");
        mActionNewFile->setText("New Script");
        mActionToggleMinimap->setText("Toggle Minimap");
        mSearchAction->setText("Search");

        setToolbarButtonsEnabled(true);

        connect(mActionOpenFile, &Action::triggered, this, &PythonEditor::handleActionOpenFile);
        connect(mActionSave, &Action::triggered, this, &PythonEditor::handleActionSaveFile);
        connect(mActionSaveAs, &Action::triggered, this, &PythonEditor::handleActionSaveFileAs);
        connect(mActionRun, &Action::triggered, this, &PythonEditor::handleActionRun);
        connect(mActionNewFile, &Action::triggered, this, &PythonEditor::handleActionNewTab);
        connect(mActionToggleMinimap, &Action::triggered, this, &PythonEditor::handleActionToggleMinimap);
        connect(mSearchAction, &QAction::triggered, this, &PythonEditor::toggleSearchbar);

        connect(mSearchbar, &Searchbar::triggerNewSearch, this, &PythonEditor::updateSearchIcon);
        connect(mSearchbar, &Searchbar::triggerNewSearch, this, &PythonEditor::handleSearchbarTextEdited);

        connect(mTabWidget, &QTabWidget::currentChanged, this, &PythonEditor::handleCurrentTabChanged);

        connect(FileManager::get_instance(), &FileManager::fileOpened, this, &PythonEditor::handleFileOpened);
        connect(FileManager::get_instance(), &FileManager::fileAboutToClose, this, &PythonEditor::handleFileAboutToClose);

        mPathEditorMap = QMap<QString, PythonCodeEditor*>();

        mFileModifiedBar = new FileModifiedBar();
        mFileModifiedBar->setHidden(true);
        mContentLayout->addWidget(mFileModifiedBar);
        connect(mFileModifiedBar, &FileModifiedBar::reloadClicked, this, &PythonEditor::handleBaseFileModifiedReload);
        connect(mFileModifiedBar, &FileModifiedBar::ignoreClicked, this, &PythonEditor::handleBaseFileModifiedIgnore);
        connect(mFileModifiedBar, &FileModifiedBar::okClicked, this, &PythonEditor::handleBaseFileModifiedOk);

        mFileWatcher = new QFileSystemWatcher(this);
        connect(mFileWatcher, &QFileSystemWatcher::fileChanged, this, &PythonEditor::handleTabFileChanged);
        connect(mFileWatcher, &QFileSystemWatcher::fileChanged, mFileModifiedBar, &FileModifiedBar::handleFileChanged);

        mSettingFontSize = new SettingsItemSpinbox("Font Size", "python/font_size", 11, "Python Editor", "Size of Font measured in pt");
        mSettingFontSize->setRange(6, 48);

        mSettingLineNumbers = new SettingsItemCheckbox("Line Numbers", "python/line_numbers", true, "Python Editor", "Enables line numbers.");

        mSettingHighlight = new SettingsItemCheckbox("Highlight Current Lines", "python/highlight_current_line", true, "Python Editor", "The current line in the editor gets highlighted if enabled.");

        mSettingLineWrap = new SettingsItemCheckbox("Line Wrap", "python/line_wrap", false, "Python Editor", "Autowraps lines in the editor to prevent horizontal scroll bars.");

        mSettingMinimap = new SettingsItemCheckbox("Code Minimap", "python/minimap", false, "Python Editor", "Enable code minimap.");

        mSettingOpenFile = new SettingsItemKeybind(
            "PyEditor Shortcut 'Open Python File'", "keybinds/python_open_file", QKeySequence("Ctrl+Shift+O"), "Keybindings: PyEditor", "Keybind for opening a python file in the Python Editor.");

        mSettingSaveFile = new SettingsItemKeybind(
            "PyEditor Shortcut 'Save Python File'", "keybinds/python_save_file", QKeySequence("Ctrl+Shift+S"), "Keybindings: PyEditor", "Keybind for saving a python file in the Python Editor.");

        mSettingSaveFileAs = new SettingsItemKeybind("PyEditor Shortcut 'Save Python File As'",
                                                     "keybinds/python_save_file_as",
                                                     QKeySequence("Ctrl+Alt+S"),
                                                     "Keybindings: PyEditor",
                                                     "Keybind for saving a python file in the Python Editor 'as ...' in the Python Editor.");

        mSettingRunFile = new SettingsItemKeybind(
            "PyEditor Shortcut 'Run Python File'", "keybinds/python_run_file", QKeySequence("Ctrl+R"), "Keybindings: PyEditor", "Keybind for executing a python file in the Python Editor.");

        mSettingCreateFile = new SettingsItemKeybind("PyEditor Shortcut 'Create New Python File'",
                                                     "keybinds/python_create_file",
                                                     QKeySequence("Ctrl+Shift+N"),
                                                     "Keybindings: PyEditor",
                                                     "Keybind for creating a new python file in the Python Editor.");

        handleActionNewTab();

        using namespace std::placeholders;
    }

    bool PythonEditor::handleDeserializationFromHalFile(const std::filesystem::path& path, Netlist* netlist, rapidjson::Document& document)
    {
        UNUSED(path);

        QString netlist_name = QString::fromStdString(netlist->get_design_name());
        // Replace all special characters from the netlist name to avoid file system problems
        netlist_name.replace("/", "-");
        netlist_name.replace("\\", "-");

        int cnt = 0;
        if (document.HasMember("PythonEditor"))
        {
            auto root = document["PythonEditor"].GetObject();

            auto array = root["tabs"].GetArray();
            for (auto it = array.Begin(); it != array.End(); ++it)
            {
                cnt++;
                if (mTabWidget->count() < cnt)
                {
                    handleActionNewTab();
                }
                auto val = it->GetObject();

                if (val.HasMember("path"))
                {
                    QFileInfo original_path(val["path"].GetString());

                    tabLoadFile(cnt - 1, original_path.filePath());
                }
            }
            if (root.HasMember("selected_tab"))
            {
                mTabWidget->setCurrentIndex(root["selected_tab"].GetInt());
            }
        }

        return true;
    }

    void PythonEditor::handleTabCloseRequested(int index)
    {
        PythonCodeEditor* editor = dynamic_cast<PythonCodeEditor*>(mTabWidget->widget(index));
        QString relFilename        = editor->getRelFilename();
        if (editor->document()->isModified())
        {
            QMessageBox::StandardButton ret = askSaveTab(index);

            if (ret == QMessageBox::Cancel)
                return;

            // Remove existing snapshots
            removeSnapshotFile(editor);

            // discard is not handled specially, we just treat the document
            // as if it did not require saving and call discardTab on it

            if (ret == QMessageBox::Save)
            {
                if (relFilename.isEmpty())
                {
                    bool suc = saveFile(false, QueryAlways, index);
                    if (!suc)
                        return;
                }
                else
                    saveFile(false, QueryIfEmpty, index);
            }
            this->discardTab(index);
        }
        else
        {
            this->discardTab(index);
        }
        saveControl();
    }

    void PythonEditor::handleActionToggleMinimap()
    {
        if (mTabWidget->currentWidget())
            dynamic_cast<PythonCodeEditor*>(mTabWidget->currentWidget())->toggleMinimap();
    }

    void PythonEditor::handleKeyPressed()
    {
        mLastClickTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    void PythonEditor::handleTextChanged()
    {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - 100 < mLastClickTime)
        {
            PythonCodeEditor* currentEditor = dynamic_cast<PythonCodeEditor*>(mTabWidget->currentWidget());

            QString tab_name = mTabWidget->tabText(mTabWidget->indexOf(currentEditor));

            if (currentEditor)
                gFileStatusManager->fileChanged(currentEditor->getUuid(), "Python tab: " + tab_name);

            if (!tab_name.endsWith("*"))
                mTabWidget->setTabText(mTabWidget->indexOf(currentEditor), tab_name + "*");
        }
    }

    void PythonEditor::handleSearchbarTextEdited(const QString& text, SearchOptions opts)
    {
        if (mTabWidget->count() > 0)
            dynamic_cast<PythonCodeEditor*>(mTabWidget->currentWidget())->search(text, opts);

        if (mSearchbar->filterApplied())
            mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchActiveIconStyle, mSearchIconPath));
        else
            mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchIconStyle, mSearchIconPath));
    }

    void PythonEditor::handleCurrentTabChanged(int index)
    {
        Q_UNUSED(index)

        bool enable = mTabWidget->count() > 0;

        QStringList iconPath, iconStyle;

        QAction* entryBasedAction[] = {mActionSave, mActionSaveAs, mActionRun, mActionToggleMinimap, mSearchAction, nullptr};

        iconStyle << mSaveIconStyle << mSaveAsIconStyle << mRunIconStyle << mToggleMinimapIconStyle << mSearchIconStyle;
        iconPath << mSaveIconPath << mSaveAsIconPath << mRunIconPath << mToggleMinimapIconPath << mSearchIconPath;

        for (int iacc = 0; entryBasedAction[iacc]; iacc++)
        {
            entryBasedAction[iacc]->setEnabled(enable);
            entryBasedAction[iacc]->setIcon(gui_utility::getStyledSvgIcon(enable ? iconStyle.at(iacc) : disabledIconStyle(), iconPath.at(iacc)));
        }

        if (!mTabWidget->currentWidget())
        {
            mSearchbar->hide();
            return;
        }

        PythonCodeEditor* currentEditor = dynamic_cast<PythonCodeEditor*>(mTabWidget->currentWidget());

        if (!mSearchbar->isHidden())
            currentEditor->search(mSearchbar->getCurrentText());
        else if (!currentEditor->extraSelections().isEmpty())
            currentEditor->search("");

        if (currentEditor->isBaseFileModified() || (gPythonContext->pythonThread()))
            mFileModifiedBar->setHidden(false);
        else
            mFileModifiedBar->setHidden(true);

        updateSearchIcon();
    }

    PythonEditor::~PythonEditor()
    {
    }

    void PythonEditor::setupToolbar(Toolbar* Toolbar)
    {
        Toolbar->addAction(mActionNewFile);
        Toolbar->addAction(mActionOpenFile);
        Toolbar->addAction(mActionSave);
        Toolbar->addAction(mActionSaveAs);
        Toolbar->addAction(mActionRun);
        Toolbar->addAction(mActionToggleMinimap);
        Toolbar->addAction(mSearchAction);
    }

    QList<QShortcut*> PythonEditor::createShortcuts()
    {
        QShortcut* shortcutNewFile    = new QShortcut(mSettingCreateFile->value().toString(), this);
        QShortcut* shortcutOpenFile   = new QShortcut(mSettingOpenFile->value().toString(), this);
        QShortcut* shortcutSaveFile   = new QShortcut(mSettingSaveFile->value().toString(), this);
        QShortcut* shortcutSaveFileAs = new QShortcut(mSettingSaveFileAs->value().toString(), this);
        QShortcut* shortcutRun        = new QShortcut(mSettingRunFile->value().toString(), this);
        mSearchShortcut               = new QShortcut(mSearchKeysequence, this);

        connect(mSearchShortcut, &QShortcut::activated, mSearchAction, &QAction::trigger);
        connect(shortcutNewFile, &QShortcut::activated, mActionNewFile, &QAction::trigger);
        connect(shortcutOpenFile, &QShortcut::activated, mActionOpenFile, &QAction::trigger);
        connect(shortcutSaveFile, &QShortcut::activated, mActionSave, &QAction::trigger);
        connect(shortcutSaveFileAs, &QShortcut::activated, mActionSaveAs, &QAction::trigger);
        connect(shortcutRun, &QShortcut::activated, mActionRun, &QAction::trigger);

        connect(mSettingCreateFile, &SettingsItemKeybind::keySequenceChanged, shortcutNewFile, &QShortcut::setKey);
        connect(mSettingOpenFile, &SettingsItemKeybind::keySequenceChanged, shortcutOpenFile, &QShortcut::setKey);
        connect(mSettingSaveFile, &SettingsItemKeybind::keySequenceChanged, shortcutSaveFile, &QShortcut::setKey);
        connect(mSettingSaveFileAs, &SettingsItemKeybind::keySequenceChanged, shortcutSaveFileAs, &QShortcut::setKey);
        connect(mSettingRunFile, &SettingsItemKeybind::keySequenceChanged, shortcutRun, &QShortcut::setKey);

        QList<QShortcut*> list;
        list.append(shortcutNewFile);
        list.append(shortcutOpenFile);
        list.append(shortcutSaveFile);
        list.append(shortcutSaveFileAs);
        list.append(shortcutRun);
        list.append(mSearchShortcut);

        return list;
    }

    void PythonEditor::handleStdout(const QString& output)
    {
        Q_EMIT forwardStdout(output);
    }

    void PythonEditor::handleError(const QString& output)
    {
        Q_EMIT forwardError(output);
    }

    void PythonEditor::clear()
    {
    }

    void PythonEditor::handleActionOpenFile()
    {
        QString caption = "Open File";
        QString filter = "Python Scripts(*.py)";

        // Non native dialogs does not work on macOS. Therefore do net set DontUseNativeDialog!
        QStringList file_names = QFileDialog::getOpenFileNames(nullptr, caption, getDefaultPath(), filter);

        if (file_names.isEmpty())
        {
            return;
        }

        for (auto fileName : file_names)
        {
            for (int i = 0; i < mTabWidget->count(); ++i)
            {
                auto editor = dynamic_cast<PythonCodeEditor*>(mTabWidget->widget(i));
                if (editor->getAbsFilename() == fileName)
                {
                    mTabWidget->setCurrentIndex(i);

                    if (editor->document()->isModified())
                    {
                        if (QMessageBox::question(editor, "Script has unsaved changes", "Do you want to reload the file from disk? Unsaved changes are lost.", QMessageBox::Yes | QMessageBox::No)
                            == QMessageBox::Yes)
                        {
                            tabLoadFile(i, fileName);
                        }
                    }
                    return;
                }
            }

            handleActionNewTab();
            tabLoadFile(mTabWidget->count() - 1, fileName);
        }

        mDefaultPath = QFileInfo(file_names.last()).path();
    }

    void PythonEditor::tabLoadFile(u32 index, QString fileName)
    {
        QFile pyFile(fileName);
        if (!pyFile.open(QIODevice::ReadOnly))
            return;

        QByteArray pyText = pyFile.readAll();
        QFileInfo info(fileName);

        while (mTabWidget->count() <= (int)index)
            handleActionNewTab();

        auto tab = dynamic_cast<PythonCodeEditor*>(mTabWidget->widget(index));

        tab->setPlainText(QString::fromUtf8(pyText));

        if (!info.fileName().startsWith(".unnamed_tab"))
        {
            tab->document()->setModified(false);
            tab->setFilename(fileName);
            mTabWidget->setTabText(mTabWidget->indexOf(tab), info.completeBaseName() + "." + info.completeSuffix());
            mNewFileCounter--;

            mPathEditorMap.insert(fileName, tab);
            mFileWatcher->addPath(fileName);

            gFileStatusManager->fileSaved(tab->getUuid());
        }
    }

    QString PythonEditor::getDefaultPath() const
    {
        if (!mDefaultPath.isEmpty()) return mDefaultPath;
        ProjectManager* pm = ProjectManager::instance();
        if (pm->get_project_status() != ProjectManager::ProjectStatus::NONE)
            return QString::fromStdString(pm->get_project_directory().get_filename("py").string());
        return QDir::currentPath();
    }

    PythonCodeEditor* PythonEditor::getPythonEditor(int tabIndex)
    {
        return dynamic_cast<PythonCodeEditor*>(mTabWidget->widget(tabIndex));
    }

    bool PythonEditor::saveFile(bool isAutosave, PythonEditor::QueryFilenamePolicy queryPolicy, int index)
    {
        QString title = "Save File";
        QString filter = "Python Scripts(*.py)";

        QString selected_file_name;

        bool isUnnamed = false;

        if (index == -1)
        {
            index = mTabWidget->currentIndex();
        }

        PythonCodeEditor* currentEditor = getPythonEditor(index);
        if (!currentEditor)
            return false;

        // currentFilename : unnamed -> empty      autosave -> into autosave
        QString currentFilename = currentEditor->getAbsFilename();
        if (isAutosave && !currentFilename.isEmpty())
        {
            currentFilename = QDir(mGenericPath).absoluteFilePath(autosaveFilename(index));
        }

        bool changeFileLocation = false;

        // evaluate query policy
        if (queryPolicy == QueryAlways ||
                ( queryPolicy == QueryIfEmpty && currentFilename.isEmpty()))
        {
            selected_file_name = QFileDialog::getSaveFileName(this, title, getDefaultPath(), filter, nullptr, QFileDialog::DontUseNativeDialog);
            if (selected_file_name.isEmpty())
                return false;

            if (!selected_file_name.endsWith(".py"))
                selected_file_name.append(".py");

            changeFileLocation = true;
        }
        else if (queryPolicy == GenericName && currentFilename.isEmpty())
        {
            selected_file_name = QDir(mGenericPath).absoluteFilePath(unnamedFilename(index));
            isUnnamed = true;
        }
        else
        {
            selected_file_name = currentFilename;
            // Remove an existing snapshot
            removeSnapshotFile(currentEditor);
        }

        std::ofstream out(selected_file_name.toStdString(), std::ios::out);
        if (!out.is_open())
        {
            log_error("gui", "could not open file path '{}' to serialize python script", selected_file_name.toStdString());
            QMessageBox::warning(this, "Save Script Error", "Cannot save python script to\n<" + selected_file_name + ">");
            return false;
        }

        if (changeFileLocation)
        {

            currentEditor->setFilename(selected_file_name);
            mDefaultPath = QFileInfo(selected_file_name).path();

            // Remove an existing snapshot and update its location
            removeSnapshotFile(currentEditor);
            QString snapShotDirectory = getSnapshotDirectory(true);
            if (!snapShotDirectory.isEmpty())
            {
                QString new_snapshot_path = snapShotDirectory + "/" + selected_file_name + ".py";
                if(mTabToSnapshotPath.contains(currentEditor))
                {
                    mTabToSnapshotPath[currentEditor] = new_snapshot_path;
                }
                else
                {
                    mTabToSnapshotPath.insert(currentEditor, new_snapshot_path);
                }
            }
        }

        bool isFileWatched = mFileWatcher->files().contains(currentFilename);
        if (isFileWatched)
        {
            mFileWatcher->removePath(currentFilename);
            mPathEditorMap.remove(currentFilename);
        }

        out << currentEditor->toPlainText().toStdString();
        out.close();

        if (!isAutosave)
        {
            currentEditor->document()->setModified(false);
            gFileStatusManager->fileSaved(currentEditor->getUuid());
        }

        if (isFileWatched)
        {
            mPathEditorMap.insert(selected_file_name, currentEditor);
            mFileWatcher->addPath(selected_file_name);
        }

        QFileInfo info(selected_file_name);
        if (!isAutosave) {
            if (isUnnamed)
            {
                QString unnamedTabName = mTabWidget->tabText(index);
                while (unnamedTabName.endsWith("*")) unnamedTabName.chop(1);
                mTabWidget->setTabText(index, unnamedTabName);
            }
            else
                mTabWidget->setTabText(index, info.completeBaseName() + "." + info.completeSuffix());
        }

        return true;
    }

    QString PythonEditor::unnamedFilename(int index) const
    {
        return QString(".unnamed_tab%1.py").arg(index);
    }

    QString PythonEditor::autosaveFilename(int index)
    {
        PythonCodeEditor* pce = getPythonEditor(index);
        if (!pce || pce->getRelFilename().isEmpty()) return unnamedFilename(index);
        return QString(".autosave_tab%1_%2").arg(index).arg(QFileInfo(pce->getRelFilename()).fileName());
    }

    void PythonEditor::saveAllTabs(const QString& genericPath, bool isAutosave)
    {
        mGenericPath = genericPath;
        QDir().mkpath(mGenericPath);
        QStringList retval;
        for (int inx=0; inx<mTabWidget->count(); ++inx)
            saveFile(isAutosave,GenericName,inx);
    }

    void PythonEditor::saveControl()
    {
        mSerializer.serialize_control();
    }

    QTabWidget* PythonEditor::getTabWidget()
    {
        return mTabWidget;
    }

    void PythonEditor::discardTab(int index)
    {
        PythonCodeEditor* editor = dynamic_cast<PythonCodeEditor*>(mTabWidget->widget(index));
        QString absFilename      = editor->getAbsFilename();
        if (!absFilename.isEmpty())
        {
            mFileWatcher->removePath(absFilename);
            mPathEditorMap.remove(absFilename);
        }
        if (editor->document()->isModified())
        {
            gFileStatusManager->fileSaved(editor->getUuid());
        }
        mTabWidget->removeTab(index);
    }

    bool PythonEditor::confirmDiscardForRange(int start, int end, int exclude)
    {
        QString changedFiles = "The following files have not been saved yet:\n";
        int unsaved          = 0;
        int total            = end - start - (exclude == -1 ? 0 : 1);
        for (int t = start; t < end; t++)
        {
            // to disable, set exclude=-1
            if (t == exclude)
                continue;

            PythonCodeEditor* editor = dynamic_cast<PythonCodeEditor*>(mTabWidget->widget(t));
            if (editor->document()->isModified())
            {
                QString fileName = mTabWidget->tabText(t);
                fileName.chop(1);    // removes asterisk
                changedFiles.append("   ->  " + fileName + "\n");
                unsaved++;
            }
        }
        if (unsaved)
        {
            QMessageBox msgBox;
            msgBox.setStyleSheet("QLabel{min-width: 600px;}");
            auto cancelButton = msgBox.addButton("Cancel", QMessageBox::RejectRole);
            msgBox.addButton("Close Anyway", QMessageBox::ApplyRole);
            msgBox.setDefaultButton(cancelButton);
            msgBox.setInformativeText(QStringLiteral("Are you sure you want to close %1 tabs, %2 unsaved, anyway?").arg(total).arg(unsaved));
            msgBox.setText("There are unsaved modifications that will be lost.");
            msgBox.setDetailedText(changedFiles);

            msgBox.exec();
            if (msgBox.clickedButton() == cancelButton)
            {
                return false;
            }
        }
        return true;
    }

    void PythonEditor::setToolbarButtonsEnabled(bool enable)
    {
        mActionSave->setEnabled(enable);
        mActionSaveAs->setEnabled(enable);
        mActionRun->setEnabled(enable);
        mActionToggleMinimap->setEnabled(enable);
        mSearchAction->setEnabled(enable);
    }

    void PythonEditor::handleActionSaveFile()
    {
        this->saveFile(false, QueryIfEmpty);
        saveControl();
    }

    void PythonEditor::handleActionSaveFileAs()
    {
        this->saveFile(false, QueryAlways);
        saveControl();
    }

    void PythonEditor::handleActionRun()
    {
        if (!mFileModifiedBar->isHidden())
        {
            QMessageBox::warning(this, "Script execution error", "Please respond to code editor message before executing the script");
            return;
        }

        mFileModifiedBar->handleScriptExecute(mTabWidget->tabText(mTabWidget->currentIndex()));
        mFileModifiedBar->setHidden(false);

        // Update snapshots when clicking on run
        this->updateSnapshots();

        for (GraphContext* ctx : gGraphContextManager->getContexts())
        {
            mBlockedContextIds.append(ctx->id());
            ctx->beginChange();
        }

        gPythonContext->interpretScript(this,dynamic_cast<PythonCodeEditor*>(mTabWidget->currentWidget())->toPlainText());
    }

    void PythonEditor::handleThreadFinished()
    {
        for (u32 ctxId : mBlockedContextIds)
        {
            GraphContext* ctx = gGraphContextManager->getContextById(ctxId);
            if (ctx) ctx->endChange();
        }
        mBlockedContextIds.clear();

        mFileModifiedBar->setHidden(true);
    }

    void PythonEditor::handleActionNewTab()
    {
        PythonCodeEditor* editor = new PythonCodeEditor();
        editor->setFontSize(mSettingFontSize->value().toInt());
        editor->setMinimapEnabled(mSettingMinimap->value().toBool());
        editor->setLineWrapEnabled(mSettingLineWrap->value().toBool());
        editor->setHighlightCurrentLineEnabled(mSettingHighlight->value().toBool());
        editor->setLineNumberEnabled(mSettingLineNumbers->value().toBool());

        connect(mSettingFontSize, &SettingsItemSpinbox::intChanged, editor, &CodeEditor::setFontSize);
        connect(mSettingMinimap, &SettingsItemCheckbox::boolChanged, editor, &CodeEditor::setMinimapEnabled);
        connect(mSettingLineWrap, &SettingsItemCheckbox::boolChanged, editor, &CodeEditor::setLineWrapEnabled);
        connect(mSettingHighlight, &SettingsItemCheckbox::boolChanged, editor, &CodeEditor::setHighlightCurrentLineEnabled);
        connect(mSettingLineNumbers, &SettingsItemCheckbox::boolChanged, editor, &CodeEditor::setLineNumberEnabled);

        new PythonSyntaxHighlighter(editor->document());
        new PythonSyntaxHighlighter(editor->minimap()->document());
        mTabWidget->addTab(editor, QString("Unnamed Script ").append(QString::number(++mNewFileCounter)));
        mTabWidget->setCurrentIndex(mTabWidget->count() - 1);
        editor->document()->setModified(false);
        connect(editor, &PythonCodeEditor::keyPressed, this, &PythonEditor::handleKeyPressed);
        connect(editor, &PythonCodeEditor::textChanged, this, &PythonEditor::handleTextChanged);
    }

    void PythonEditor::handleActionTabMenu()
    {
        QMenu context_menu(this);
        QAction* action = context_menu.addAction("Close");

        context_menu.addSeparator();
        connect(action, &QAction::triggered, this, &PythonEditor::handleActionCloseTab);
        action = context_menu.addAction("Close all");
        connect(action, &QAction::triggered, this, &PythonEditor::handleActionCloseAllTabs);
        action = context_menu.addAction("Close all others");
        connect(action, &QAction::triggered, this, &PythonEditor::handleActionCloseOtherTabs);
        action = context_menu.addAction("Close all right");
        connect(action, &QAction::triggered, this, &PythonEditor::handleActionCloseRightTabs);
        action = context_menu.addAction("Close all left");
        connect(action, &QAction::triggered, this, &PythonEditor::handleActionCloseLeftTabs);

        context_menu.addSeparator();
        action = context_menu.addAction("Save");
        connect(action, &QAction::triggered, this, &PythonEditor::handleActionSaveFile);
        action = context_menu.addAction("Save as");
        connect(action, &QAction::triggered, this, &PythonEditor::handleActionSaveFileAs);

        context_menu.addSeparator();
        action                   = context_menu.addAction("Show in system explorer");
        PythonCodeEditor* editor = dynamic_cast<PythonCodeEditor*>(mTabWidget->widget(mTabRightclicked));
        QString absFilename      = editor->getAbsFilename();
        action->setData(absFilename);
        action->setDisabled(absFilename.isEmpty());
        connect(action, &QAction::triggered, this, &PythonEditor::handleActionShowFile);

        context_menu.exec(QCursor::pos());
    }

    void PythonEditor::handleActionCloseTab()
    {
        assert(mTabRightclicked != -1);
        this->handleTabCloseRequested(mTabRightclicked);
    }

    void PythonEditor::handleActionCloseAllTabs()
    {
        assert(mTabRightclicked != -1);
        int tabs = mTabWidget->count();
        if (!this->confirmDiscardForRange(0, tabs))
            return;
        for (int t = 0; t < tabs; t++)
        {
            this->discardTab(0);
        }
    }

    void PythonEditor::handleActionCloseOtherTabs()
    {
        assert(mTabRightclicked != -1);
        int tabs = mTabWidget->count();
        if (!this->confirmDiscardForRange(0, tabs, mTabRightclicked))
            return;
        int discard_id = 0;    // keeps track of IDs shifting during deletion
        for (int t = 0; t < tabs; t++)
        {
            // don't close the right-clicked tab
            if (t == mTabRightclicked)
            {
                discard_id++;
                continue;
            }
            this->discardTab(discard_id);
        }
    }

    void PythonEditor::handleActionCloseLeftTabs()
    {
        assert(mTabRightclicked != -1);
        if (!this->confirmDiscardForRange(0, mTabRightclicked, -1))
            return;
        for (int t = 0; t < mTabRightclicked; t++)
        {
            // IDs shift downwards during deletion
            this->discardTab(0);
        }
    }

    void PythonEditor::handleActionCloseRightTabs()
    {
        assert(mTabRightclicked != -1);
        int tabs = mTabWidget->count();
        if (!this->confirmDiscardForRange(mTabRightclicked + 1, tabs, -1))
            return;
        for (int t = mTabRightclicked + 1; t < tabs; t++)
        {
            // IDs shift downwards during deletion
            this->discardTab(mTabRightclicked + 1);
        }
    }

    void PythonEditor::handleActionShowFile()
    {
        QAction* action = dynamic_cast<QAction*>(sender());
        if (!action)
        {
            log_error("gui", "could not cast sender into QAction.");
            return;
        }

        //the data is set in the handleActionTabMenu (the path of the underlying file)
        QFileInfo info(action->data().toString());
        if (!info.exists())
        {
            log_error("gui", "File does not exist.");
            return;
        }

        QDesktopServices::openUrl(QUrl(info.absolutePath(), QUrl::TolerantMode));
    }

    void PythonEditor::handleTabFileChanged(QString path)
    {
        PythonCodeEditor* editor_with_modified_base_file = mPathEditorMap.value(path);
        editor_with_modified_base_file->setBaseFileModified(true);
        int tabIndex = mTabWidget->indexOf(editor_with_modified_base_file);
        QString tab_name = mTabWidget->tabText(tabIndex);

        if (!tab_name.endsWith("*"))
            mTabWidget->setTabText(tabIndex, tab_name + "*");

        gFileStatusManager->fileChanged(editor_with_modified_base_file->getUuid(), "Python tab: " + tab_name);

        PythonCodeEditor* currentEditor = getPythonEditor(tabIndex);

        if (editor_with_modified_base_file == currentEditor)
            mFileModifiedBar->setHidden(false);

        mFileWatcher->addPath(path);
    }

    void PythonEditor::handleBaseFileModifiedReload()
    {
        PythonCodeEditor* currentEditor = dynamic_cast<PythonCodeEditor*>(mTabWidget->currentWidget());
        mNewFileCounter++;
        //tabLoadFile(currentEditor, currentEditor->getFileName());
        tabLoadFile(mTabWidget->indexOf(currentEditor), currentEditor->getAbsFilename());
        currentEditor->setBaseFileModified(false);
        mFileModifiedBar->setHidden(true);
    }

    void PythonEditor::handleBaseFileModifiedIgnore()
    {
        PythonCodeEditor* currentEditor = dynamic_cast<PythonCodeEditor*>(mTabWidget->currentWidget());
        currentEditor->setBaseFileModified(false);
        mFileModifiedBar->setHidden(true);
    }

    void PythonEditor::handleBaseFileModifiedOk()
    {
        PythonCodeEditor* currentEditor = dynamic_cast<PythonCodeEditor*>(mTabWidget->currentWidget());
        currentEditor->setBaseFileModified(false);
        mFileModifiedBar->setHidden(true);
    }

    void PythonEditor::handleFileOpened(QString fileName)
    {
        // Check for snapshots and load them if available
        QPair<QMap<QString, QString>, QVector<QString>> snapshots = this->loadAllSnapshots();
        QMap<QString, QString> saved_snapshots                    = snapshots.first;
        QVector<QString> unsaved_snapshots                        = snapshots.second;

        if (saved_snapshots.isEmpty() && unsaved_snapshots.isEmpty())
        {
            // No snapshots found. Nothing to do.
            return;
        }

        if (!fileName.endsWith(".hal"))
        {
            // The .v/.vhdl file was parsed again.
            bool deleteSnapshots = askDeleteSnapshots(snapshots);
            if (deleteSnapshots)
            {
                // Delete and ignore all snapshots
                clearAllSnapshots(true);
                return;
            }
            else
            {
                // The empty tab is closed to open all unstored snapshots
                if (mTabWidget->count() == 1)
                {
                    this->discardTab(0);
                }
            }
        }

        int tabs = mTabWidget->count();
        // Handle the tabs that were already opened (by the infos from the .hal file)
        for (int idx = 0; idx < tabs; idx++)
        {
            PythonCodeEditor* editor = dynamic_cast<PythonCodeEditor*>(mTabWidget->widget(idx));
            QFileInfo original_path(editor->getAbsFilename());

            // Decide whether the snapshot file or the original should be loaded
            bool load_snapshot = decideLoadSnapshot(saved_snapshots, original_path);

            if (load_snapshot)
            {
                this->setSnapshotContent(idx, saved_snapshots[original_path.absoluteFilePath()]);
            }
            saved_snapshots.remove(original_path.absoluteFilePath());
        }

        // Handle paths that did not appear int the .hal file
        for (auto snapshot_original_path : saved_snapshots.keys())
        {
            QFileInfo original_path(snapshot_original_path);
            bool load_snapshot = decideLoadSnapshot(saved_snapshots, original_path);
            handleActionNewTab();
            int tab_idx = mTabWidget->count() - 1;
            tabLoadFile(tab_idx, original_path.filePath());
            if (load_snapshot)
            {
                this->setSnapshotContent(tab_idx, saved_snapshots[snapshot_original_path]);
            }
            //(load_snapshot) ? tabLoadFile(tab_idx, original_path.filePath()) : setSnapshotContent(tab_idx, saved_snapshots[snapshot_original_path]);
        }

        // Load snapshots of unsaved tabs
        for (QString snapshot_content : unsaved_snapshots)
        {
            this->handleActionNewTab();
            this->setSnapshotContent(mTabWidget->count() - 1, snapshot_content);
        }
        updateSnapshots();
    }

    void PythonEditor::handleFileAboutToClose(const QString& fileName)
    {
        Q_UNUSED(fileName)
        clearAllSnapshots(true);

        //clear all open tabs and reset the edior
        while (mTabWidget->count() > 0)
            discardTab(0);
        mNewFileCounter = 0;
        mLastClickTime  = 0;
        handleActionNewTab();
    }

    void PythonEditor::updateSearchIcon()
    {
        if (mSearchbar->filterApplied() && mSearchbar->isVisible())
            mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchActiveIconStyle, mSearchIconPath));
        else if (!mSearchAction->isEnabled())
            mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mDisabledIconStyle, mSearchIconPath));
        else
            mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchIconStyle, mSearchIconPath));
    }

    bool PythonEditor::eventFilter(QObject* obj, QEvent* event)
    {
        if (obj == mTabWidget->tabBar() && event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            // filter for right-mouse-button-pressed events
            if (mouseEvent->button() == Qt::MouseButton::RightButton)
            {
                mTabRightclicked = mTabWidget->tabBar()->tabAt(mouseEvent->pos());
                this->handleActionTabMenu();
                return true;
            }
        }
        // otherwise honor default filter
        return QObject::eventFilter(obj, event);
    }

    QPair<QString, QString> PythonEditor::readSnapshotFile(QFileInfo snapshot_file_path) const
    {
        QFile snapshot_file(snapshot_file_path.absoluteFilePath());
        if (!snapshot_file.open(QIODevice::ReadOnly))
        {
            log_error("gui", "Cannot open snapshot file {}!", snapshot_file_path.absoluteFilePath().toStdString());
            return QPair<QString, QString>("", "");
        }

        QTextStream stream(&snapshot_file);
        QString file_content       = stream.readAll();
        QString original_file_path = file_content.section('\n', 0, 0);
        QString snapshot_content   = file_content.section('\n', 1);

        return QPair<QString, QString>(original_file_path, snapshot_content);
    }

    QPair<QMap<QString, QString>, QVector<QString>> PythonEditor::loadAllSnapshots()
    {
        QString snapshot_path = this->getSnapshotDirectory(false);

        if (snapshot_path.isEmpty())
        {
            return QPair<QMap<QString, QString>, QVector<QString>>();
        }
        QMap<QString, QString> stored_snapshot_map;
        QVector<QString> unstored_snapshots;
        QDir snapshot_dir(snapshot_path);

        QStringList snapshot_files = snapshot_dir.entryList(QStringList() << "*.py", QDir::Files);

        for (QString snapshot_file_name : snapshot_files)
        {
            QString snapshot_file_path                        = snapshot_dir.absoluteFilePath(snapshot_file_name);
            QPair<QString, QString> original_path_and_content = this->readSnapshotFile(snapshot_file_path);
            QString original_path                             = original_path_and_content.first;
            if (original_path.isEmpty() || (!QFileInfo(original_path).exists()))
            {
                // Original File does not exist
                unstored_snapshots.append(original_path_and_content.second);
            }
            else
            {
                // An original file exists
                stored_snapshot_map.insert(original_path_and_content.first, original_path_and_content.second);
            }
        }

        return QPair<QMap<QString, QString>, QVector<QString>>(stored_snapshot_map, unstored_snapshots);
    }

    bool PythonEditor::writeSnapshotFile(QFileInfo snapshot_file_path, QString original_file_path, QString content) const
    {
        QFile snapshot_file(snapshot_file_path.filePath());
        if (!snapshot_file.open(QIODevice::WriteOnly))
        {
            log_error("gui", "Cannot open snapshot file to write!");
            return false;
        }

        snapshot_file.write(original_file_path.toUtf8());
        snapshot_file.write("\n");
        snapshot_file.write(content.toUtf8());
        snapshot_file.close();

        return true;
    }

    QString PythonEditor::getSnapshotDirectory(const bool create_if_non_existent)
    {
        if (!FileManager::get_instance()->fileName().isEmpty())
        {
            QFileInfo info(FileManager::get_instance()->fileName());
            QDir snapshot_dir    = info.absoluteDir();
            QString completePath = snapshot_dir.absolutePath() + "/~" + info.baseName();

            if (!snapshot_dir.exists(completePath))
            {
                if (create_if_non_existent)
                {
                    if (!snapshot_dir.mkpath(completePath))
                    {
                        log_error("gui", "Failed to create snapshot directory.");
                        return "";
                    }
                }
                else
                    return "";
            }
            return completePath;
        }

        return "";
    }

    void PythonEditor::updateSnapshots()
    {
        this->clearAllSnapshots(false);
        mTabToSnapshotPath.clear();

        QDir snapshot_dir = this->getSnapshotDirectory(true);

        int tabs = mTabWidget->count();
        for (int index = 0; index < tabs; index++)
        {
            PythonCodeEditor* editor   = dynamic_cast<PythonCodeEditor*>(mTabWidget->widget(index));
            QString snapshot_file_name = "~";
            if (editor->getRelFilename().isEmpty())
            {
                // The Tab is unstored
                snapshot_file_name += "unsaved_tab";
            }
            else
            {
                // An original file exists
                QFileInfo original_file_name(editor->getRelFilename());
                snapshot_file_name += original_file_name.fileName();
            }
            //if the filename ends with .py because it is loaded from (or saved to) a file, insert the tabindex before the file extension, otherwise append the index and extension
            snapshot_file_name.endsWith(".py") ? snapshot_file_name.insert(snapshot_file_name.length() - 3, "__(" + QString::number(index) + ")__")
                                               : snapshot_file_name += "__(" + QString::number(index) + ")__.py";

            QString snapshot_file_path = snapshot_dir.absoluteFilePath(snapshot_file_name);

            mTabToSnapshotPath.insert(editor, snapshot_file_path);

            if (editor->document()->isModified())
            {
                // The snapshot is only created if there are modifications
                this->writeSnapshotFile(snapshot_file_path, editor->getAbsFilename(), editor->toPlainText());
            }
        }
    }

    void PythonEditor::clearAllSnapshots(bool remove_dir)
    {
        QString snapshot_dir_path = this->getSnapshotDirectory(false);

        if (snapshot_dir_path == "")
        {
            // The directory does not exist. Nothing to do then
            return;
        }
        QDir snapshot_dir(snapshot_dir_path);

        // Make sure the directory is a snapshot directory (additional protection)
        if (!snapshot_dir.dirName().startsWith('~'))
        {
            log_error("gui", "Can not delete directory: '{}' is not a snapshot directory!", snapshot_dir.absolutePath().toStdString());
            return;
        }

        if (remove_dir)
        {
            snapshot_dir.removeRecursively();
        }
        else
        {
            // Remove all .py files in the directory
            snapshot_dir.setNameFilters(QStringList() << "*.py");
            snapshot_dir.setFilter(QDir::Files);
            for (QString dirFile : snapshot_dir.entryList())
            {
                snapshot_dir.remove(dirFile);
            }
        }
    }

    bool PythonEditor::decideLoadSnapshot(const QMap<QString, QString>& saved_snapshots, const QFileInfo original_path) const
    {
        bool load_snapshot = false;
        if (saved_snapshots.contains(original_path.absoluteFilePath()))
        {
            if (original_path.exists())
            {
                // Ask user whether the original file or the snapshot should be loaded
                QString original_content;
                QFile original_file(original_path.absoluteFilePath());
                if (!original_file.open(QIODevice::ReadOnly))
                {
                    original_content = "Cannot open original file...";
                }
                else
                {
                    original_content = QString::fromStdString(original_file.readAll().toStdString());
                }
                load_snapshot = askLoadSnapshot(original_path.absoluteFilePath(), original_content, saved_snapshots[original_path.absoluteFilePath()]);
            }
            else
            {
                load_snapshot = true;
            }
        }
        return load_snapshot;
    }

    void PythonEditor::setSnapshotContent(const int idx, const QString snapshot_content)
    {
        if (idx < 0 || idx >= mTabWidget->count())
        {
            log_error("gui", "Cannot insert snapshot content is tab. Index {} is out of range.", idx);
        }
        PythonCodeEditor* tab = dynamic_cast<PythonCodeEditor*>(mTabWidget->widget(idx));
        // Set the snapshot content
        tab->setPlainText(snapshot_content);
        // Mark the tab as modified
        tab->document()->setModified(true);
        QString tab_name = mTabWidget->tabText(idx);
        if (!tab_name.endsWith("*"))
        {
            tab_name += "*";
            mTabWidget->setTabText(idx, tab_name);
        }
        gFileStatusManager->fileChanged(tab->getUuid(), "Python tab: " + tab_name);
    }

    bool PythonEditor::askLoadSnapshot(const QString original_path, const QString original_content, const QString snapshot_content) const
    {
        QMessageBox msgBox((QWidget*)nullptr);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle("Python snapshot file detected");
        msgBox.setText("A snapshot file (for " + original_path
                       + ") was found! This may happen due to a recent crash.\n"
                         "Do you want to load the snapshot file or the (unsaved) original file?");
        auto load_snapshot_btn = msgBox.addButton("Load Snapshot", QMessageBox::ActionRole);
        msgBox.addButton("Load Original", QMessageBox::ActionRole);

        // Details
        QString detailed_text = "";

        detailed_text = "=== Original File (" + original_path + ") ===\n" + original_content + "\n=== Snapshot File ===\n" + snapshot_content;

        msgBox.setDetailedText(detailed_text);

        QSpacerItem* horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        QGridLayout* layout           = (QGridLayout*)msgBox.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

        msgBox.exec();

        if (msgBox.clickedButton() == (QAbstractButton*)load_snapshot_btn)
        {
            return true;
        }
        return false;
    }

    bool PythonEditor::askDeleteSnapshots(const QPair<QMap<QString, QString>, QVector<QString>>& snapshots) const
    {
        QMessageBox msgBox((QWidget*)nullptr);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle("Python snapshot file detected");
        msgBox.setText("You have just parsed an hdl file but there are still old snapshot files left. This may happen due to a recent crash. "
                       "Do you want to ignore and delete all old snapshot files and stay with their last saved state? "
                       "Or do you want to open the old python scripts to save them properly?");
        auto delete_snapshots_btn = msgBox.addButton("Delete and Ignore Snapshots", QMessageBox::ActionRole);
        msgBox.addButton("Open Old Scripts", QMessageBox::ActionRole);

        // Details
        QString detailed_text = "";

        detailed_text = "Snapshot files of the following paths were found:";
        for (auto origPath : snapshots.first.keys())
        {
            detailed_text += "\n'" + origPath + "'";
        }
        if (!snapshots.second.isEmpty())
        {
            detailed_text += "\n+ " + QString::number(snapshots.second.size()) + " unsaved tabs";
        }

        msgBox.setDetailedText(detailed_text);

        QSpacerItem* horizontalSpacer = new QSpacerItem(800, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        QGridLayout* layout           = (QGridLayout*)msgBox.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

        msgBox.exec();

        if (msgBox.clickedButton() == (QAbstractButton*)delete_snapshots_btn)
        {
            return true;
        }
        return false;
    }

    QMessageBox::StandardButton PythonEditor::askSaveTab(const int tab_index) const
    {
        QMessageBox msgBox;
        msgBox.setStyleSheet("QLabel{min-width: 600px;}");
        msgBox.setText(mTabWidget->tabText(tab_index).append(" has been modified."));
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        return static_cast<QMessageBox::StandardButton>(msgBox.exec());
    }

    void PythonEditor::removeSnapshotFile(PythonCodeEditor* editor) const
    {
        if (!mTabToSnapshotPath.contains(editor))
        {
            return;
        }

        QFileInfo snapshot_path(mTabToSnapshotPath[editor]);
        if (snapshot_path.exists())
        {
            QFile snapshot_file(snapshot_path.filePath());
            snapshot_file.remove();
        }
    }

    QString PythonEditor::disabledIconStyle() const
    {
        return mDisabledIconStyle;
    }

    QString PythonEditor::openIconPath() const
    {
        return mOpenIconPath;
    }

    QString PythonEditor::openIconStyle() const
    {
        return mOpenIconStyle;
    }

    QString PythonEditor::saveIconPath() const
    {
        return mSaveIconPath;
    }

    QString PythonEditor::saveIconStyle() const
    {
        return mSaveIconStyle;
    }

    QString PythonEditor::saveAsIconPath() const
    {
        return mSaveAsIconPath;
    }

    QString PythonEditor::saveAsIconStyle() const
    {
        return mSaveAsIconStyle;
    }

    QString PythonEditor::runIconPath() const
    {
        return mRunIconPath;
    }

    QString PythonEditor::runIconStyle() const
    {
        return mRunIconStyle;
    }

    QString PythonEditor::newFileIconPath() const
    {
        return mNewFileIconPath;
    }

    QString PythonEditor::newFileIconStyle() const
    {
        return mNewFileIconStyle;
    }

    QString PythonEditor::toggleMinimapIconPath() const
    {
        return mToggleMinimapIconPath;
    }

    QString PythonEditor::toggleMinimapIconStyle() const
    {
        return mToggleMinimapIconStyle;
    }

    void PythonEditor::setDisabledIconStyle(const QString& style)
    {
        mDisabledIconStyle = style;
    }

    void PythonEditor::setOpenIconPath(const QString& path)
    {
        mOpenIconPath = path;
    }

    void PythonEditor::setOpenIconStyle(const QString& style)
    {
        mOpenIconStyle = style;
    }

    void PythonEditor::setSaveIconPath(const QString& path)
    {
        mSaveIconPath = path;
    }

    void PythonEditor::setSaveIconStyle(const QString& style)
    {
        mSaveIconStyle = style;
    }

    void PythonEditor::setSaveAsIconPath(const QString& path)
    {
        mSaveAsIconPath = path;
    }

    void PythonEditor::setSaveAsIconStyle(const QString& style)
    {
        mSaveAsIconStyle = style;
    }

    void PythonEditor::setRunIconPath(const QString& path)
    {
        mRunIconPath = path;
    }

    void PythonEditor::setRunIconStyle(const QString& style)
    {
        mRunIconStyle = style;
    }

    void PythonEditor::setNewFileIconPath(const QString& path)
    {
        mNewFileIconPath = path;
    }

    void PythonEditor::setNewFileIconStyle(const QString& style)
    {
        mNewFileIconStyle = style;
    }

    void PythonEditor::setToggleMinimapIconPath(const QString& path)
    {
        mToggleMinimapIconPath = path;
    }

    void PythonEditor::setToggleMinimapIconStyle(const QString& style)
    {
        mToggleMinimapIconStyle = style;
    }

    void PythonEditor::toggleSearchbar()
    {
        if (!mSearchAction->isEnabled())
            return;

        if (mSearchbar->isHidden())
        {
            mSearchbar->show();
            mSearchbar->setFocus();
        }
        else
        {
            mSearchbar->hide();
            if (mTabWidget->currentWidget())
                mTabWidget->currentWidget()->setFocus();
            else
                this->setFocus();
        }
    }

    QTextDocument::FindFlags PythonEditor::getFindFlags()
    {
        QTextDocument::FindFlags options = QTextDocument::FindFlags();
        if (mSearchbar->getSearchOptions().isCaseSensitive())
            options = options | QTextDocument::FindCaseSensitively;
        if (mSearchbar->getSearchOptions().isExactMatch())
            options = options | QTextDocument::FindWholeWords;
        return options;
    }

    QString PythonEditor::searchIconPath() const
    {
        return mSearchIconPath;
    }

    QString PythonEditor::searchIconStyle() const
    {
        return mSearchIconStyle;
    }

    QString PythonEditor::searchActiveIconStyle() const
    {
        return mSearchActiveIconStyle;
    }

    void PythonEditor::setSearchIconPath(const QString& path)
    {
        mSearchIconPath = path;
    }

    void PythonEditor::setSearchIconStyle(const QString& style)
    {
        mSearchIconStyle = style;
    }

    void PythonEditor::setSearchActiveIconStyle(const QString& style)
    {
        mSearchActiveIconStyle = style;
    }
}    // namespace hal
