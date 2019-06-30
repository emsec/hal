#include "python/python_editor.h"
#include "python/python_code_editor.h"
#include "python/python_editor_code_completion_dialog.h"

#include "code_editor/syntax_highlighter/python_syntax_highlighter.h"
#include "core/log.h"

#include "gui_globals.h"
#include "gui_utility.h"
#include "searchbar/searchbar.h"
#include "splitter/splitter.h"
#include "toolbar/toolbar.h"

#include <QAction>
#include <QFileDialog>
#include <QShortcut>
#include <QTextDocumentFragment>
#include <QTextStream>
#include <QToolButton>
#include <QVBoxLayout>

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <chrono>
#include <fstream>

python_editor::python_editor(QWidget* parent)
    : content_widget("Python Editor", parent), python_context_subscriber(), m_searchbar(new searchbar()), m_action_open_file(new QAction(this)), m_action_run(new QAction(this)),
      m_action_save(new QAction(this)), m_action_save_as(new QAction(this)), m_action_toggle_minimap(new QAction(this)), m_action_new_file(new QAction(this))
{
    ensurePolished();
    m_new_file_counter = 0;
    m_last_click_time  = 0;

    m_tab_widget = new QTabWidget(this);
    m_tab_widget->setTabsClosable(true);
    m_tab_widget->setMovable(true);
    m_content_layout->addWidget(m_tab_widget);
    connect(m_tab_widget, &QTabWidget::tabCloseRequested, this, &python_editor::handle_tab_close_requested);
    m_content_layout->addWidget(m_searchbar);
    m_searchbar->hide();

    m_action_open_file->setIcon(gui_utility::get_styled_svg_icon(m_open_icon_style, m_open_icon_path));
    m_action_save->setIcon(gui_utility::get_styled_svg_icon(m_save_icon_style, m_save_icon_path));
    m_action_save_as->setIcon(gui_utility::get_styled_svg_icon(m_save_as_icon_style, m_save_as_icon_path));
    m_action_run->setIcon(gui_utility::get_styled_svg_icon(m_run_icon_style, m_run_icon_path));
    m_action_toggle_minimap->setIcon(gui_utility::get_styled_svg_icon(m_toggle_minimap_icon_style, m_toggle_minimap_icon_path));
    m_action_new_file->setIcon(gui_utility::get_styled_svg_icon(m_new_file_icon_style, m_new_file_icon_path));

    m_action_open_file->setShortcut(QKeySequence("Ctrl+Shift+O"));
    m_action_save->setShortcut(QKeySequence("Shift+Ctrl+S"));
    m_action_save_as->setShortcut(QKeySequence("Alt+Ctrl+S"));
    m_action_run->setShortcut(QKeySequence("Ctrl+R"));
    m_action_new_file->setShortcut(QKeySequence("Ctrl+Shift+n"));

    m_action_open_file->setText("Open Script '" + m_action_open_file->shortcut().toString(QKeySequence::NativeText) + "'");
    m_action_save->setText("Save '" + m_action_save->shortcut().toString(QKeySequence::NativeText) + "'");
    m_action_save_as->setText("Save as '" + m_action_save->shortcut().toString(QKeySequence::NativeText) + "'");
    m_action_run->setText("Execute Script '" + m_action_run->shortcut().toString(QKeySequence::NativeText) + "'");
    m_action_new_file->setText("New File '" + m_action_new_file->shortcut().toString(QKeySequence::NativeText) + "'");
    m_action_toggle_minimap->setText("Toggle Minimap");

    connect(m_action_open_file, &QAction::triggered, this, &python_editor::handle_action_open_file);
    connect(m_action_save, &QAction::triggered, this, &python_editor::handle_action_save_file);
    connect(m_action_save_as, &QAction::triggered, this, &python_editor::handle_action_save_file_as);
    connect(m_action_run, &QAction::triggered, this, &python_editor::handle_action_run);
    connect(m_action_new_file, &QAction::triggered, this, &python_editor::handle_action_new_tab);
    connect(m_action_toggle_minimap, &QAction::triggered, this, &python_editor::handle_action_toggle_minimap);

    connect(m_searchbar, &searchbar::text_edited, this, &python_editor::handle_searchbar_text_edited);
    connect(m_tab_widget, &QTabWidget::currentChanged, this, &python_editor::handle_current_tab_changed);

    handle_action_new_tab();

    using namespace std::placeholders;
    hal_file_manager::register_on_serialize_callback("python_editor", std::bind(&python_editor::handle_serialization_to_hal_file, this, _1, _2, _3));
    hal_file_manager::register_on_deserialize_callback("python_editor", std::bind(&python_editor::handle_deserialization_from_hal_file, this, _1, _2, _3));
}

bool python_editor::handle_serialization_to_hal_file(const hal::path& path, std::shared_ptr<netlist> netlist, rapidjson::Document& document)
{
    UNUSED(path);
    UNUSED(netlist);

    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    rapidjson::Value tabs(rapidjson::kArrayType);

    for (int i = 0; i < m_tab_widget->count(); i++)
    {
        rapidjson::Value val(rapidjson::kObjectType);

        auto tab = dynamic_cast<python_code_editor*>(m_tab_widget->widget(i));

        if (!tab->get_file_name().isEmpty())
        {
            val.AddMember("path", tab->get_file_name().toStdString(), allocator);
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
        if (m_tab_widget->currentIndex() != -1)
        {
            root.AddMember("selected_tab", rapidjson::Value(m_tab_widget->currentIndex()), allocator);
        }
        document.AddMember("python_editor", root, allocator);
    }
    return true;
}

bool python_editor::handle_deserialization_from_hal_file(const hal::path& path, std::shared_ptr<netlist> netlist, rapidjson::Document& document)
{
    UNUSED(path);
    UNUSED(netlist);

    if (document.HasMember("python_editor"))
    {
        auto root  = document["python_editor"].GetObject();
        int cnt    = 0;
        auto array = root["tabs"].GetArray();
        for (auto it = array.Begin(); it != array.End(); ++it)
        {
            cnt++;
            if (m_tab_widget->count() < cnt)
            {
                handle_action_new_tab();
            }

            auto val = it->GetObject();

            if (val.HasMember("path"))
            {
                tab_load_file(cnt - 1, val["path"].GetString());
            }

            if (val.HasMember("script"))
            {
                auto tab = dynamic_cast<python_code_editor*>(m_tab_widget->widget(cnt - 1));
                tab->setPlainText(val["script"].GetString());
                tab->document()->setModified(true);
            }
        }

        if (root.HasMember("selected_tab"))
        {
            m_tab_widget->setCurrentIndex(root["selected_tab"].GetInt());
        }
    }
    return true;
}

void python_editor::handle_tab_close_requested(int index)
{
    python_code_editor* editor = dynamic_cast<python_code_editor*>(m_tab_widget->widget(index));
    if (editor->document()->isModified())
    {
        QMessageBox msgBox;
        msgBox.setStyleSheet("QLabel{min-width: 600px;}");
        msgBox.setText(m_tab_widget->tabText(index).append(" has been modified."));
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Cancel)
            return;

        if (ret == QMessageBox::Discard)
        {
            m_tab_widget->removeTab(index);
            return;
        }

        if (ret == QMessageBox::Save)
        {
            if (editor->get_file_name().isEmpty())
                save_file(true, index);
            else
                save_file(false, index);

            m_tab_widget->removeTab(index);
            return;
        }
    }
    m_tab_widget->removeTab(index);
}

void python_editor::handle_action_toggle_minimap()
{
    if (m_tab_widget->currentWidget())
        dynamic_cast<python_code_editor*>(m_tab_widget->currentWidget())->toggle_minimap();
}

void python_editor::handle_modification_changed(bool changed)
{
    if (changed && !(m_tab_widget->tabText(m_tab_widget->currentIndex()).endsWith("*")))
    {
        m_tab_widget->setTabText(m_tab_widget->currentIndex(), m_tab_widget->tabText(m_tab_widget->currentIndex()).append("*"));
    }
    if (!changed && (m_tab_widget->tabText(m_tab_widget->currentIndex()).endsWith("*")))
    {
        m_tab_widget->setTabText(m_tab_widget->currentIndex(), m_tab_widget->tabText(m_tab_widget->currentIndex()).remove('*'));
    }
}

void python_editor::handle_key_pressed()
{
    m_last_click_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void python_editor::handle_text_changed()
{
    if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - 100 < m_last_click_time)
    {
        g_content_manager.data_changed("Python editor tab " + QString::number(m_tab_widget->currentIndex() + 1));
    }
}

void python_editor::handle_searchbar_text_edited(const QString& text)
{
    if (m_tab_widget->count() > 0)
        dynamic_cast<python_code_editor*>(m_tab_widget->currentWidget())->search(text);
}

void python_editor::handle_current_tab_changed(int index)
{
    Q_UNUSED(index)

    if (!m_tab_widget->currentWidget())
        return;

    python_code_editor* current_editor = dynamic_cast<python_code_editor*>(m_tab_widget->currentWidget());

    if (!m_searchbar->isHidden())
        current_editor->search(m_searchbar->get_current_text());
    else if (!current_editor->extraSelections().isEmpty())
        current_editor->search("");
}

python_editor::~python_editor()
{
    hal_file_manager::unregister_on_serialize_callback("python_editor");
    hal_file_manager::unregister_on_deserialize_callback("python_editor");
}

void python_editor::setup_toolbar(toolbar* toolbar)
{
    toolbar->addAction(m_action_new_file);
    toolbar->addAction(m_action_open_file);
    toolbar->addAction(m_action_save);
    toolbar->addAction(m_action_save_as);
    toolbar->addAction(m_action_run);
    toolbar->addAction(m_action_toggle_minimap);
}

QList<QShortcut*> python_editor::create_shortcuts()
{
    QShortcut* search_shortcut = new QShortcut(QKeySequence("Ctrl+f"), this);
    connect(search_shortcut, &QShortcut::activated, this, &python_editor::toggle_searchbar);

    QList<QShortcut*> list;
    list.append(search_shortcut);

    return list;
}

void python_editor::handle_stdout(const QString& output)
{
    Q_EMIT forward_stdout(output);
}

void python_editor::handle_error(const QString& output)
{
    Q_EMIT forward_error(output);
}

void python_editor::clear()
{
}

void python_editor::handle_action_open_file()
{
    QString title = "Open File";
    QString text  = "Python Scripts(*.py)";

    QStringList file_names = QFileDialog::getOpenFileNames(nullptr, title, QDir::currentPath(), text, nullptr, QFileDialog::DontUseNativeDialog);

    if (file_names.isEmpty())
    {
        return;
    }

    for(auto file_name : file_names)
    {
        for (int i = 0; i < m_tab_widget->count(); ++i)
        {
            auto editor = dynamic_cast<python_code_editor*>(m_tab_widget->widget(i));
            if (editor->get_file_name() == file_name)
            {
                m_tab_widget->setCurrentIndex(i);

                if (editor->document()->isModified())
                {
                    if (QMessageBox::question(editor, "Script has unsaved changes", "Do you want to reload the file from disk? Unsaved changes are lost.", QMessageBox::Yes | QMessageBox::No)
                        == QMessageBox::Yes)
                    {
                        tab_load_file(i, file_name);
                    }
                }
                return;
            }
        }

        handle_action_new_tab();
        tab_load_file(m_tab_widget->count() - 1, file_name);
    }
}

void python_editor::tab_load_file(u32 index, QString file_name)
{
    std::ifstream file(file_name.toStdString(), std::ios::in);

    if (!file.is_open())
    {
        return;
    }

    std::string f((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    QFileInfo info(file_name);

    auto tab = dynamic_cast<python_code_editor*>(m_tab_widget->widget(index));

    tab->setPlainText(QString::fromStdString(f));
    tab->set_file_name(file_name);
    tab->document()->setModified(false);
    m_tab_widget->setTabText(m_tab_widget->count() - 1, info.completeBaseName() + "." + info.completeSuffix());
    m_new_file_counter--;

    g_content_manager.data_saved("Python editor tab " + QString::number(index + 1));
}

void python_editor::save_file(const bool ask_path, int index)
{
    QString title = "Save File";
    QString text  = "Python Scripts(*.py)";

    QString selected_file_name;

    if (index == -1)
    {
        index = m_tab_widget->currentIndex();
    }

    python_code_editor* current_editor = dynamic_cast<python_code_editor*>(m_tab_widget->widget(index));

    if (!current_editor)
        return;

    if (ask_path || current_editor->get_file_name().isEmpty())
    {
        selected_file_name = QFileDialog::getSaveFileName(nullptr, title, QDir::currentPath(), text, nullptr, QFileDialog::DontUseNativeDialog);
        if (selected_file_name.isEmpty())
            return;

        if (!selected_file_name.endsWith(".py"))
            selected_file_name.append(".py");

        current_editor->set_file_name(selected_file_name);
    }
    else
    {
        selected_file_name = current_editor->get_file_name();
    }

    std::ofstream out(selected_file_name.toStdString(), std::ios::out);

    if (!out.is_open())
    {
        log_error("gui", "could not open file path");
        return;
    }
    out << current_editor->toPlainText().toStdString();
    out.close();
    current_editor->document()->setModified(false);
    g_content_manager.data_saved("Python editor tab " + QString::number(index + 1));

    QFileInfo info(selected_file_name);
    m_tab_widget->setTabText(index, info.completeBaseName() + "." + info.completeSuffix());
}

void python_editor::handle_action_save_file()
{
    this->save_file(false);
}

void python_editor::handle_action_save_file_as()
{
    this->save_file(true);
}

void python_editor::handle_action_run()
{
    g_python_context->interpret_script(dynamic_cast<python_code_editor*>(m_tab_widget->currentWidget())->toPlainText());
}

void python_editor::handle_action_new_tab()
{
    python_code_editor* editor = new python_code_editor();
    new python_syntax_highlighter(editor->document());
    new python_syntax_highlighter(editor->minimap()->document());
    m_tab_widget->addTab(editor, QString("New File ").append(QString::number(++m_new_file_counter)));
    m_tab_widget->setCurrentIndex(m_tab_widget->count() - 1);
    editor->document()->setModified(false);
    connect(editor, &python_code_editor::modificationChanged, this, &python_editor::handle_modification_changed);
    connect(editor, &python_code_editor::key_pressed, this, &python_editor::handle_key_pressed);
    connect(editor, &python_code_editor::textChanged, this, &python_editor::handle_text_changed);
}

QString python_editor::open_icon_path() const
{
    return m_open_icon_path;
}

QString python_editor::open_icon_style() const
{
    return m_open_icon_style;
}

QString python_editor::save_icon_path() const
{
    return m_save_icon_path;
}

QString python_editor::save_icon_style() const
{
    return m_save_icon_style;
}

QString python_editor::save_as_icon_path() const
{
    return m_save_as_icon_path;
}

QString python_editor::save_as_icon_style() const
{
    return m_save_as_icon_style;
}

QString python_editor::run_icon_path() const
{
    return m_run_icon_path;
}

QString python_editor::run_icon_style() const
{
    return m_run_icon_style;
}

QString python_editor::new_file_icon_path() const
{
    return m_new_file_icon_path;
}

QString python_editor::new_file_icon_style() const
{
    return m_new_file_icon_style;
}

QString python_editor::toggle_minimap_icon_path() const
{
    return m_toggle_minimap_icon_path;
}

QString python_editor::toggle_minimap_icon_style() const
{
    return m_toggle_minimap_icon_style;
}

void python_editor::set_open_icon_path(const QString& path)
{
    m_open_icon_path = path;
}

void python_editor::set_open_icon_style(const QString& style)
{
    m_open_icon_style = style;
}

void python_editor::set_save_icon_path(const QString& path)
{
    m_save_icon_path = path;
}

void python_editor::set_save_icon_style(const QString& style)
{
    m_save_icon_style = style;
}

void python_editor::set_save_as_icon_path(const QString& path)
{
    m_save_as_icon_path = path;
}

void python_editor::set_save_as_icon_style(const QString& style)
{
    m_save_as_icon_style = style;
}

void python_editor::set_run_icon_path(const QString& path)
{
    m_run_icon_path = path;
}

void python_editor::set_run_icon_style(const QString& style)
{
    m_run_icon_style = style;
}

void python_editor::set_new_file_icon_path(const QString& path)
{
    m_new_file_icon_path = path;
}

void python_editor::set_new_file_icon_style(const QString& style)
{
    m_new_file_icon_style = style;
}

void python_editor::set_toggle_minimap_icon_path(const QString& path)
{
    m_toggle_minimap_icon_path = path;
}

void python_editor::set_toggle_minimap_icon_style(const QString& style)
{
    m_toggle_minimap_icon_style = style;
}

void python_editor::toggle_searchbar()
{
    if (m_searchbar->isHidden())
    {
        m_searchbar->show();
        if (m_tab_widget->currentWidget())
            dynamic_cast<python_code_editor*>(m_tab_widget->currentWidget())->search(m_searchbar->get_current_text());
        m_searchbar->setFocus();
    }
    else
    {
        m_searchbar->hide();
        if (m_tab_widget->currentWidget())
        {
            dynamic_cast<python_code_editor*>(m_tab_widget->currentWidget())->search("");
            m_tab_widget->currentWidget()->setFocus();
        }
        else
            this->setFocus();
    }
}
