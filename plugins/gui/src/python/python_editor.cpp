#include "gui/python/python_editor.h"

#include "gui/code_editor/syntax_highlighter/python_syntax_highlighter.h"
#include "hal_core/utilities/log.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"
#include "gui/action/action.h"
#include "gui/python/python_code_editor.h"
#include "gui/python/python_editor_code_completion_dialog.h"
#include "gui/searchbar/searchbar.h"
#include "gui/splitter/splitter.h"
#include "gui/toolbar/toolbar.h"

#include <QAction>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QShortcut>
#include <QTabBar>
#include <QTextDocumentFragment>
#include <QTextStream>
#include <QToolButton>
#include <QVBoxLayout>
#include <chrono>
#include <fstream>

namespace hal
{
    PythonEditor::PythonEditor(QWidget* parent)
        : ContentWidget("Python Editor", parent), PythonContextSubscriber(), mSearchbar(new Searchbar()), mActionOpenFile(new Action(this)), mActionRun(new Action(this)),
          mActionSave(new Action(this)), mActionSaveAs(new Action(this)), mActionToggleMinimap(new Action(this)), mActionNewFile(new Action(this))
    {
        ensurePolished();
        mNewFileCounter = 0;
        mLastClickTime  = 0;

        mLastOpenedPath = QDir::currentPath();

        mTabWidget = new QTabWidget(this);
        mTabWidget->setTabsClosable(true);
        mTabWidget->setMovable(true);
        // we need to grab mouse events from the tab bar
        mTabWidget->tabBar()->installEventFilter(this);
        mContentLayout->addWidget(mTabWidget);
        connect(mTabWidget, &QTabWidget::tabCloseRequested, this, &PythonEditor::handleTabCloseRequested);
        mContentLayout->addWidget(mSearchbar);
        mSearchbar->hide();

        mActionOpenFile->setIcon(gui_utility::getStyledSvgIcon(mOpenIconStyle, mOpenIconPath));
        mActionSave->setIcon(gui_utility::getStyledSvgIcon(mSaveIconStyle, mSaveIconPath));
        mActionSaveAs->setIcon(gui_utility::getStyledSvgIcon(mSaveAsIconStyle, mSaveAsIconPath));
        mActionRun->setIcon(gui_utility::getStyledSvgIcon(mRunIconStyle, mRunIconPath));
        mActionToggleMinimap->setIcon(gui_utility::getStyledSvgIcon(mToggleMinimapIconStyle, mToggleMinimapIconPath));
        mActionNewFile->setIcon(gui_utility::getStyledSvgIcon(mNewFileIconStyle, mNewFileIconPath));

        gKeybindManager->bind(mActionOpenFile, "keybinds/python_open_file");
        gKeybindManager->bind(mActionSave, "keybinds/python_save_file");
        gKeybindManager->bind(mActionSaveAs, "keybinds/python_save_file_as");
        gKeybindManager->bind(mActionRun, "keybinds/python_run_file");
        gKeybindManager->bind(mActionNewFile, "keybinds/python_create_file");

        mActionOpenFile->setText("Open Script");
        mActionSave->setText("Save");
        mActionSaveAs->setText("Save as");
        mActionRun->setText("Execute Script");
        mActionNewFile->setText("New File");
        mActionToggleMinimap->setText("Toggle Minimap");

        connect(mActionOpenFile, &Action::triggered, this, &PythonEditor::handleActionOpenFile);
        connect(mActionSave, &Action::triggered, this, &PythonEditor::handleActionSaveFile);
        connect(mActionSaveAs, &Action::triggered, this, &PythonEditor::handleActionSaveFileAs);
        connect(mActionRun, &Action::triggered, this, &PythonEditor::handleActionRun);
        connect(mActionNewFile, &Action::triggered, this, &PythonEditor::handleActionNewTab);
        connect(mActionToggleMinimap, &Action::triggered, this, &PythonEditor::handleActionToggleMinimap);

        connect(mSearchbar, &Searchbar::textEdited, this, &PythonEditor::handleSearchbarTextEdited);
        connect(mTabWidget, &QTabWidget::currentChanged, this, &PythonEditor::handleCurrentTabChanged);

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

        handleActionNewTab();

        using namespace std::placeholders;
        hal_file_manager::register_on_serialize_callback("PythonEditor", std::bind(&PythonEditor::handleSerializationToHalFile, this, _1, _2, _3));
        hal_file_manager::register_on_deserialize_callback("PythonEditor", std::bind(&PythonEditor::handleDeserializationFromHalFile, this, _1, _2, _3));
    }

    bool PythonEditor::handleSerializationToHalFile(const std::filesystem::path& path, Netlist* netlist, rapidjson::Document& document)
    {
        UNUSED(path);
        UNUSED(netlist);

        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
        rapidjson::Value tabs(rapidjson::kArrayType);

        for (int i = 0; i < mTabWidget->count(); i++)
        {
            rapidjson::Value val(rapidjson::kObjectType);

            auto tab = dynamic_cast<PythonCodeEditor*>(mTabWidget->widget(i));

            if (!tab->getFileName().isEmpty())
            {
                val.AddMember("path", tab->getFileName().toStdString(), allocator);
            }

            if (tab->document()->isModified())
            {
                val.AddMember("script", tab->toPlainText().toStdString(), allocator);
            }

            tabs.PushBack(val, allocator);
        }

        if (!tabs.Empty())
        {
            rapidjson::Value root(rapidjson::kObjectType);
            root.AddMember("tabs", tabs, allocator);
            if (mTabWidget->currentIndex() != -1)
            {
                root.AddMember("selected_tab", rapidjson::Value(mTabWidget->currentIndex()), allocator);
            }
            document.AddMember("PythonEditor", root, allocator);
        }
        return true;
    }

    bool PythonEditor::handleDeserializationFromHalFile(const std::filesystem::path& path, Netlist* netlist, rapidjson::Document& document)
    {
        UNUSED(path);
        UNUSED(netlist);

        if (document.HasMember("PythonEditor"))
        {
            auto root  = document["PythonEditor"].GetObject();
            int cnt    = 0;
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
                    tabLoadFile(cnt - 1, val["path"].GetString());
                }

                if (val.HasMember("script"))
                {
                    auto tab = dynamic_cast<PythonCodeEditor*>(mTabWidget->widget(cnt - 1));
                    tab->setPlainText(val["script"].GetString());
                    tab->document()->setModified(true);
                    mTabWidget->setTabText(cnt - 1, mTabWidget->tabText(cnt - 1) + "*");
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
        if (editor->document()->isModified())
        {
            QMessageBox msgBox;
            msgBox.setStyleSheet("QLabel{min-width: 600px;}");
            msgBox.setText(mTabWidget->tabText(index).append(" has been modified."));
            msgBox.setInformativeText("Do you want to save your changes?");
            msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Save);
            int ret = msgBox.exec();

            if (ret == QMessageBox::Cancel)
                return;

            // discard is not handled specially, we just treat the document
            // as if it did not require saving and call discardTab on it

            if (ret == QMessageBox::Save)
            {
                if (editor->getFileName().isEmpty())
                    saveFile(true, index);
                else
                    saveFile(false, index);

                mTabWidget->removeTab(index);
                return;
            }
        }

        this->discardTab(index);
    }

    void PythonEditor::handleActionToggleMinimap()
    {
        if (mTabWidget->currentWidget())
            dynamic_cast<PythonCodeEditor*>(mTabWidget->currentWidget())->toggleMinimap();
    }

    void PythonEditor::handleModificationChanged(bool changed)
    {
        if (changed && !(mTabWidget->tabText(mTabWidget->currentIndex()).endsWith("*")))
        {
            mTabWidget->setTabText(mTabWidget->currentIndex(), mTabWidget->tabText(mTabWidget->currentIndex()).append("*"));
        }
        if (!changed && (mTabWidget->tabText(mTabWidget->currentIndex()).endsWith("*")))
        {
            mTabWidget->setTabText(mTabWidget->currentIndex(), mTabWidget->tabText(mTabWidget->currentIndex()).remove('*'));
        }
    }

    void PythonEditor::handleKeyPressed()
    {
        mLastClickTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    void PythonEditor::handleTextChanged()
    {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - 100 < mLastClickTime)
        {
            PythonCodeEditor* current_editor = dynamic_cast<PythonCodeEditor*>(mTabWidget->currentWidget());

            QString tab_name = mTabWidget->tabText(mTabWidget->indexOf(current_editor));

            if (current_editor)
                gFileStatusManager->fileChanged(current_editor->getUuid(), "Python tab: " + tab_name);

            if (!tab_name.endsWith("*"))
                mTabWidget->setTabText(mTabWidget->indexOf(current_editor), tab_name + "*");
        }
    }

    void PythonEditor::handleSearchbarTextEdited(const QString& text)
    {
        if (mTabWidget->count() > 0)
            dynamic_cast<PythonCodeEditor*>(mTabWidget->currentWidget())->search(text);
    }

    void PythonEditor::handleCurrentTabChanged(int index)
    {
        Q_UNUSED(index)

        if (!mTabWidget->currentWidget())
            return;

        PythonCodeEditor* current_editor = dynamic_cast<PythonCodeEditor*>(mTabWidget->currentWidget());

        if (!mSearchbar->isHidden())
            current_editor->search(mSearchbar->getCurrentText());
        else if (!current_editor->extraSelections().isEmpty())
            current_editor->search("");

        if (current_editor->isBaseFileModified())
            mFileModifiedBar->setHidden(false);
        else
            mFileModifiedBar->setHidden(true);
    }

    PythonEditor::~PythonEditor()
    {
        hal_file_manager::unregister_on_serialize_callback("PythonEditor");
        hal_file_manager::unregister_on_deserialize_callback("PythonEditor");
    }

    void PythonEditor::setupToolbar(Toolbar* Toolbar)
    {
        Toolbar->addAction(mActionNewFile);
        Toolbar->addAction(mActionOpenFile);
        Toolbar->addAction(mActionSave);
        Toolbar->addAction(mActionSaveAs);
        Toolbar->addAction(mActionRun);
        Toolbar->addAction(mActionToggleMinimap);
    }

    QList<QShortcut*> PythonEditor::createShortcuts()
    {
        QShortcut* search_shortcut = new QShortcut(QKeySequence("Ctrl+f"), this);
        connect(search_shortcut, &QShortcut::activated, this, &PythonEditor::toggleSearchbar);

        QList<QShortcut*> list;
        list.append(search_shortcut);

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
        QString title = "Open File";
        QString text  = "Python Scripts(*.py)";

        // Non native dialogs does not work on macOS. Therefore do net set DontUseNativeDialog!
        QStringList file_names = QFileDialog::getOpenFileNames(nullptr, title, mLastOpenedPath, text, nullptr);

        if (file_names.isEmpty())
        {
            return;
        }

        for (auto fileName : file_names)
        {
            for (int i = 0; i < mTabWidget->count(); ++i)
            {
                auto editor = dynamic_cast<PythonCodeEditor*>(mTabWidget->widget(i));
                if (editor->getFileName() == fileName)
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

        mLastOpenedPath = QFileInfo(file_names.last()).absolutePath();
    }

    void PythonEditor::tabLoadFile(u32 index, QString fileName)
    {
        std::ifstream file(fileName.toStdString(), std::ios::in);

        if (!file.is_open())
        {
            return;
        }

        std::string f((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        QFileInfo info(fileName);

        auto tab = dynamic_cast<PythonCodeEditor*>(mTabWidget->widget(index));

        tab->setPlainText(QString::fromStdString(f));
        tab->set_file_name(fileName);
        tab->document()->setModified(false);
        //mTabWidget->setTabText(mTabWidget->count() - 1, info.completeBaseName() + "." + info.completeSuffix());
        mTabWidget->setTabText(mTabWidget->indexOf(tab), info.completeBaseName() + "." + info.completeSuffix());
        mNewFileCounter--;

        mPathEditorMap.insert(fileName, tab);
        mFileWatcher->addPath(fileName);

        gFileStatusManager->fileSaved(tab->getUuid());
    }

    void PythonEditor::saveFile(const bool ask_path, int index)
    {
        QString title = "Save File";
        QString text  = "Python Scripts(*.py)";

        QString selected_file_name;

        if (index == -1)
        {
            index = mTabWidget->currentIndex();
        }

        PythonCodeEditor* current_editor = dynamic_cast<PythonCodeEditor*>(mTabWidget->widget(index));

        if (!current_editor)
            return;

        if (ask_path || current_editor->getFileName().isEmpty())
        {
            selected_file_name = QFileDialog::getSaveFileName(nullptr, title, mLastOpenedPath, text, nullptr, QFileDialog::DontUseNativeDialog);
            if (selected_file_name.isEmpty())
                return;

            if (!selected_file_name.endsWith(".py"))
                selected_file_name.append(".py");

            current_editor->set_file_name(selected_file_name);
            mLastOpenedPath = selected_file_name;
        }
        else
        {
            selected_file_name = current_editor->getFileName();
        }

        mFileWatcher->removePath(current_editor->getFileName());
        mPathEditorMap.remove(current_editor->getFileName());

        std::ofstream out(selected_file_name.toStdString(), std::ios::out);

        if (!out.is_open())
        {
            log_error("gui", "could not open file path");
            return;
        }
        out << current_editor->toPlainText().toStdString();
        out.close();
        current_editor->document()->setModified(false);

        gFileStatusManager->fileSaved(current_editor->getUuid());

        mPathEditorMap.insert(selected_file_name, current_editor);
        mFileWatcher->addPath(selected_file_name);

        QFileInfo info(selected_file_name);
        mTabWidget->setTabText(index, info.completeBaseName() + "." + info.completeSuffix());
    }

    QTabWidget *PythonEditor::getTabWidget()
    {
        return mTabWidget;
    }


    void PythonEditor::discardTab(int index)
    {
        PythonCodeEditor* editor = dynamic_cast<PythonCodeEditor*>(mTabWidget->widget(index));
        QString s                  = editor->getFileName();
        if (!s.isEmpty())
        {
            mFileWatcher->removePath(s);
            mPathEditorMap.remove(s);
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

    void PythonEditor::handleActionSaveFile()
    {
        this->saveFile(false);
    }

    void PythonEditor::handleActionSaveFileAs()
    {
        this->saveFile(true);
    }

    void PythonEditor::handleActionRun()
    {
        for (const auto& ctx : gGraphContextManager->getContexts())
        {
            ctx->beginChange();
        }

        gPythonContext->interpretScript(dynamic_cast<PythonCodeEditor*>(mTabWidget->currentWidget())->toPlainText());

        for (const auto& ctx : gGraphContextManager->getContexts())
        {
            ctx->endChange();
        }
    }

    void PythonEditor::handleActionNewTab()
    {
        PythonCodeEditor* editor = new PythonCodeEditor();
        new PythonSyntaxHighlighter(editor->document());
        new PythonSyntaxHighlighter(editor->minimap()->document());
        mTabWidget->addTab(editor, QString("New File ").append(QString::number(++mNewFileCounter)));
        mTabWidget->setCurrentIndex(mTabWidget->count() - 1);
        editor->document()->setModified(false);
        //connect(editor, &PythonCodeEditor::modificationChanged, this, &PythonEditor::handleModificationChanged);
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
        action                     = context_menu.addAction("Show in system explorer");
        PythonCodeEditor* editor = dynamic_cast<PythonCodeEditor*>(mTabWidget->widget(mTabRightclicked));
        QString s                  = editor->getFileName();
        action->setDisabled(s.isEmpty());
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
    }

    void PythonEditor::handleTabFileChanged(QString path)
    {
        PythonCodeEditor* editor_with_modified_base_file = mPathEditorMap.value(path);
        editor_with_modified_base_file->setBaseFileModified(true);
        QString tab_name = mTabWidget->tabText(mTabWidget->indexOf(editor_with_modified_base_file));

        if (!tab_name.endsWith("*"))
            mTabWidget->setTabText(mTabWidget->indexOf(editor_with_modified_base_file), tab_name + "*");

        gFileStatusManager->fileChanged(editor_with_modified_base_file->getUuid(), "Python tab: " + tab_name);

        PythonCodeEditor* current_editor = dynamic_cast<PythonCodeEditor*>(mTabWidget->currentWidget());

        if (editor_with_modified_base_file == current_editor)
            mFileModifiedBar->setHidden(false);

        mFileWatcher->addPath(path);
    }

    void PythonEditor::handleBaseFileModifiedReload()
    {
        PythonCodeEditor* current_editor = dynamic_cast<PythonCodeEditor*>(mTabWidget->currentWidget());
        mNewFileCounter++;
        //tabLoadFile(current_editor, current_editor->getFileName());
        tabLoadFile(mTabWidget->indexOf(current_editor), current_editor->getFileName());
        current_editor->setBaseFileModified(false);
        mFileModifiedBar->setHidden(true);
    }

    void PythonEditor::handleBaseFileModifiedIgnore()
    {
        PythonCodeEditor* current_editor = dynamic_cast<PythonCodeEditor*>(mTabWidget->currentWidget());
        current_editor->setBaseFileModified(false);
        mFileModifiedBar->setHidden(true);
    }

    void PythonEditor::handleBaseFileModifiedOk()
    {
        PythonCodeEditor* current_editor = dynamic_cast<PythonCodeEditor*>(mTabWidget->currentWidget());
        current_editor->setBaseFileModified(false);
        mFileModifiedBar->setHidden(true);
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
        if (mSearchbar->isHidden())
        {
            mSearchbar->show();
            if (mTabWidget->currentWidget())
                dynamic_cast<PythonCodeEditor*>(mTabWidget->currentWidget())->search(mSearchbar->getCurrentText());
            mSearchbar->setFocus();
        }
        else
        {
            mSearchbar->hide();
            if (mTabWidget->currentWidget())
            {
                dynamic_cast<PythonCodeEditor*>(mTabWidget->currentWidget())->search("");
                mTabWidget->currentWidget()->setFocus();
            }
            else
                this->setFocus();
        }
    }
}
