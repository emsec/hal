#include "python/python_editor.h"
#include "python/python_code_editor.h"
#include "python/python_editor_code_completion_dialog.h"

//#include "code_editor/code_editor_minimap.h"
#include "code_editor/syntax_highlighter/python_syntax_highlighter.h"
#include "core/log.h"
#include "gui_globals.h"
#include "gui_utility.h"
#include "searchbar/searchbar.h"
#include "splitter/splitter.h"
#include "toolbar/toolbar.h"

#include <QAction>
#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QFileDialog>
#include <QShortcut>
#include <QTextStream>
#include <QTextDocumentFragment>
#include <QToolButton>
#include <QVBoxLayout>

#include <fstream>
#include <QFileInfo>
#include <QMessageBox>

python_editor::python_editor(QWidget* parent)
    : content_widget("Python Editor", parent), python_context_subscriber(), m_editor_widget(new python_code_editor()), m_searchbar(new searchbar()), m_action_open_file(new QAction(this)),
      m_action_run(new QAction(this)), m_action_save(new QAction(this)), m_action_save_as(new QAction(this)), m_file_name("")
{
    ensurePolished();
    const int tab_stop = 4;
    m_new_file_counter = 0;

    QFontMetrics metrics(font());
    m_editor_widget->setTabStopWidth(tab_stop * metrics.width(" "));

    new python_syntax_highlighter(m_editor_widget->document());
    new python_syntax_highlighter(m_editor_widget->minimap()->document());


    m_tab_widget = new QTabWidget(this);
    m_tab_widget->setTabsClosable(true);
    m_tab_widget->addTab(m_editor_widget, "Default");
    m_content_layout->addWidget(m_tab_widget);
    connect(m_tab_widget, &QTabWidget::tabCloseRequested, this, &python_editor::handle_tab_close_requested);
    m_content_layout->addWidget(m_searchbar);
    m_searchbar->hide();

    m_action_open_file->setIcon(gui_utility::get_styled_svg_icon(m_open_icon_style, m_open_icon_path));
    m_action_save->setIcon(gui_utility::get_styled_svg_icon(m_save_icon_style, m_save_icon_path));
    m_action_save_as->setIcon(gui_utility::get_styled_svg_icon(m_save_as_icon_style, m_save_as_icon_path)); //TODO new icon
    m_action_run->setIcon(gui_utility::get_styled_svg_icon(m_run_icon_style, m_run_icon_path));

    m_action_open_file->setShortcut(QKeySequence("Ctrl+Shift+O"));
    m_action_save->setShortcut(QKeySequence("Shift+Ctrl+S"));
    m_action_save_as->setShortcut(QKeySequence("Alt+Ctrl+S"));
    m_action_run->setShortcut(QKeySequence("Ctrl+R"));

    m_action_open_file->setText("Open Script '" + m_action_open_file->shortcut().toString(QKeySequence::NativeText) + "'");
    m_action_save->setText("Save '" + m_action_save->shortcut().toString(QKeySequence::NativeText) + "'");
    m_action_save_as->setText("Save as '" + m_action_save->shortcut().toString(QKeySequence::NativeText) + "'");
    m_action_run->setText("Execute Script '" + m_action_run->shortcut().toString(QKeySequence::NativeText) + "'");

    connect(m_action_open_file, &QAction::triggered, this, &python_editor::handle_action_open_file);
    connect(m_action_save, &QAction::triggered, this, &python_editor::handle_action_save_file);
    connect(m_action_save_as, &QAction::triggered, this, &python_editor::handle_action_save_file_as);
    connect(m_action_run, &QAction::triggered, this, &python_editor::handle_action_run);


    m_editor_widget->setPlainText(g_settings.value("python_editor/code", "").toString());
    m_editor_widget->document()->setModified(false);
    connect(m_editor_widget, &code_editor::modificationChanged, this, &python_editor::handle_modification_changed);
    connect(m_searchbar, &searchbar::text_edited, this, &python_editor::handle_searchbar_text_edited);
}

void python_editor::handle_tab_close_requested(int index)
{
    python_code_editor* editor = dynamic_cast<python_code_editor*>(m_tab_widget->widget(index));
    if(editor->document()->isModified())
    {
        QMessageBox msgBox;
        msgBox.setStyleSheet("QLabel{min-width: 600px;}");
        msgBox.setText(m_tab_widget->tabText(index).append(" has been modified."));
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        if(ret == QMessageBox::Cancel)
            return;

        if(ret == QMessageBox::Discard)
        {
            m_tab_widget->removeTab(index);
            return;
        }

        if(ret == QMessageBox::Save)
        {
            if(editor->get_file_name().isEmpty())
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
    dynamic_cast<python_code_editor*>(m_tab_widget->currentWidget())->toggle_minimap();
}

void python_editor::handle_modification_changed(bool changed)
{
    if(changed && !(m_tab_widget->tabText(m_tab_widget->currentIndex()).endsWith("*")))
        m_tab_widget->setTabText(m_tab_widget->currentIndex(), m_tab_widget->tabText(m_tab_widget->currentIndex()).append("*"));
    if(!changed && (m_tab_widget->tabText(m_tab_widget->currentIndex()).endsWith("*")))
        m_tab_widget->setTabText(m_tab_widget->currentIndex(), m_tab_widget->tabText(m_tab_widget->currentIndex()).remove('*'));
}

void python_editor::handle_searchbar_text_edited(const QString &text)
{
    if(m_tab_widget->count() > 0)
        dynamic_cast<python_code_editor*>(m_tab_widget->currentWidget())->search(text);
}

python_editor::~python_editor()
{
    g_settings.setValue("python_editor/code", m_editor_widget->toPlainText());
    g_settings.sync();
}

void python_editor::setup_toolbar(toolbar* toolbar)
{
    // DEBUG CODE
    QToolButton* add_button = new QToolButton(this);
    add_button->setIcon(gui_utility::get_styled_svg_icon(m_new_file_icon_style, m_new_file_icon_path));
    add_button->setToolTip("New Tab 'CTRL+Shift+N'");
    toolbar->addWidget(add_button);
    add_button->setShortcut(QKeySequence("Ctrl+Shift+n"));
    connect(add_button, &QToolButton::clicked, this, &python_editor::handle_action_new_tab);

    toolbar->addAction(m_action_open_file);
    toolbar->addAction(m_action_save);
    toolbar->addAction(m_action_save_as);
    toolbar->addAction(m_action_run);

    // DEBUG CODE
    QToolButton* button = new QToolButton(this);
    button->setText("Debug Toggle Minimap");
    button->setIcon(gui_utility::get_styled_svg_icon("all->#FFDD00", ":/icons/placeholder"));
    button->setToolTip("Debug Toggle Minimap");

    //connect(button, &QToolButton::clicked, m_editor_widget, &code_editor::toggle_minimap);
    connect(button, &QToolButton::clicked, this, &python_editor::handle_action_toggle_minimap);

    toolbar->addWidget(button);
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

    QString new_file_name = QFileDialog::getOpenFileName(nullptr, title, QDir::currentPath(), text, nullptr, QFileDialog::DontUseNativeDialog);

    if (new_file_name.isEmpty())
    {
        return;
    }

    std::ifstream file(new_file_name.toStdString(), std::ios::in);

    if (!file.is_open())
    {
        return;
    }

    // make file active
    m_file_name = new_file_name;

    std::string f((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    QFileInfo info(new_file_name);

    handle_action_new_tab();
    python_code_editor* editor = dynamic_cast<python_code_editor*>(m_tab_widget->widget(m_tab_widget->count()-1));
    editor->appendPlainText(QString::fromStdString(f));
    editor->set_file_name(new_file_name);
    //editor->update_text_state();
    editor->document()->setModified(false);
    //editor->document()->isModified();
    m_tab_widget->setTabText(m_tab_widget->count()-1, info.completeBaseName() + "." + info.completeSuffix());
    m_new_file_counter--;
}

void python_editor::save_file(const bool ask_path, const int index)
{
    QString title = "Save File";
    QString text  = "Python Scripts(*.py)";

    QString selected_file_name;

    python_code_editor* current_editor;
    if(index == -1)
        current_editor = dynamic_cast<python_code_editor*>(m_tab_widget->currentWidget());
    else
        current_editor = dynamic_cast<python_code_editor*>(m_tab_widget->widget(index));

    if(!current_editor)
        return;

    if(ask_path || current_editor->get_file_name().isEmpty())
    {
        selected_file_name = QFileDialog::getSaveFileName(nullptr, title, QDir::currentPath(), text, nullptr, QFileDialog::DontUseNativeDialog);
        if(selected_file_name.isEmpty())
            return;

        if(!selected_file_name.endsWith(".py"))
            selected_file_name.append(".py");

        current_editor->set_file_name(selected_file_name);
    }
    else
        selected_file_name = current_editor->get_file_name();

    std::ofstream out(selected_file_name.toStdString(), std::ios::out);

    if (!out.is_open())
    {
        return;
        log_error("gui", "could not open file path");
    }
    out << current_editor->toPlainText().toStdString();
    out.close();
    current_editor->document()->setModified(false);

    QFileInfo info(selected_file_name);
    m_tab_widget->setTabText(m_tab_widget->currentIndex(), info.completeBaseName() + "." + info.completeSuffix());
    // remember target file path
    m_file_name = selected_file_name;
}

bool python_editor::has_unsaved_tabs()
{
    for(int i = 0; i < m_tab_widget->count(); i++)
    {
        if(dynamic_cast<python_code_editor*>(m_tab_widget->widget(i))->document()->isModified())
            return true;
    }
    return false;
}

QStringList python_editor::get_names_of_unsaved_tabs()
{
    QStringList unsaved_tabs;
    for(int i = 0; i < m_tab_widget->count(); i++)
    {
        if(dynamic_cast<python_code_editor*>(m_tab_widget->widget(i))->document()->isModified())
            unsaved_tabs << m_tab_widget->tabText(i);
    }
    return unsaved_tabs;
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
    m_tab_widget->setCurrentIndex(m_tab_widget->count()-1);
    editor->document()->setModified(false);
    connect(editor, &python_code_editor::modificationChanged, this, &python_editor::handle_modification_changed);
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

void python_editor::set_new_file_icon_path(const QString &path)
{
    m_new_file_icon_path = path;
}

void python_editor::set_new_file_icon_style(const QString &style)
{
    m_new_file_icon_style = style;
}

void python_editor::toggle_searchbar()
{
    if (m_searchbar->isHidden())
    {
        m_searchbar->show();
    }
    else
    {
        m_searchbar->hide();
    }
}
