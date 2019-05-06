#include "python/python_editor.h"
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
#include <QDebug>
#include <QDesktopWidget>
#include <QFile>
#include <QFileDialog>
#include <QShortcut>
#include <QTextStream>
#include <QTextDocumentFragment>
#include <QToolButton>
#include <QVBoxLayout>

#include <fstream>

void python_code_editor::keyPressEvent(QKeyEvent* e)
{
    if (textCursor().hasSelection() && !(e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab))
    {
        QPlainTextEdit::keyPressEvent(e);
        return;
    }

    if (e->key() == Qt::Key_Tab)
    {
        handle_tab_key_pressed();
        return;
    }

    if (e->key() == Qt::Key_Backtab)
    {
        handle_shift_tab_key_pressed();
        return;
    }

    if (e->key() == Qt::Key_Return)
    {
        handle_return_pressed();
        return;
    }

    if (e->key() == Qt::Key_Backspace)
    {
        handle_backspace_pressed(e);
        return;
    }

    if (e->key() == Qt::Key_Delete)
    {
        handle_delete_pressed(e);
        return;
    }

    QPlainTextEdit::keyPressEvent(e);
}

void python_code_editor::handle_shift_tab_key_pressed()
{
    python_code_editor::indent_selection(false);
}

void python_code_editor::handle_tab_key_pressed()
{
    python_code_editor::indent_selection(true);
}

void python_code_editor::handle_autocomplete()
{
    auto cursor = textCursor();
        // auto row    = cursor.blockNumber();
        // auto col    = cursor.positionInBlock();
        cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
        auto text       = cursor.selectedText().replace(QChar(0x2029), '\n');
        auto candidates = g_python_context->complete(text, false);

        if (candidates.size() == 1)
        {
            textCursor().insertText(QString::fromStdString(std::get<1>(candidates.at(0))));
        }
        else if (candidates.size() > 1)
        {
            auto dialog = new python_editor_code_completion_dialog(this, candidates);
            connect(dialog, &python_editor_code_completion_dialog::completionSelected, this, &python_code_editor::perform_code_completion);
            auto menu_width  = dialog->width();
            auto menu_height = dialog->height();

            auto desk_rect   = QApplication::desktop()->screenGeometry();
            auto desk_width  = desk_rect.width();
            auto desk_height = desk_rect.height();
            auto anchor      = this->cursorRect().bottomRight();
            anchor.setX(anchor.x() + viewportMargins().left());
            auto anchor_global = this->mapToGlobal(anchor);

            if (anchor_global.x() + menu_width > desk_width)
            {
                anchor.setX(anchor.x() - menu_width);
                anchor_global = this->mapToGlobal(anchor);
            }

            if (anchor_global.y() + menu_height > desk_height)
            {
                anchor.setY(cursorRect().topRight().y() - menu_height);
                anchor_global = this->mapToGlobal(anchor);
            }
            dialog->move(anchor_global);
            dialog->exec();
        }
}

int python_code_editor::next_indent(bool indentUnindent, int current_indent)
{
    int next_indent;
    if (indentUnindent)
    {
        next_indent = 4 - (current_indent % 4);
    }
    else
    {
        next_indent = current_indent % 4;
        if (next_indent == 0)
        {
            next_indent = 4;
        }
    }
    return next_indent;
}

void python_code_editor::indent_selection(bool indent)
{
    auto cursor = textCursor();
    bool preSelected = cursor.hasSelection();
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    if (preSelected)
    {
        // expand selection to the start of the first line
        cursor.setPosition(end, QTextCursor::MoveAnchor);
        cursor.setPosition(start, QTextCursor::KeepAnchor);
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    }
    else
    {
        // select first line
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    }
    QStringList selected_lines = cursor.selection().toPlainText().split('\n');
    cursor.clearSelection();

    // calculate number of spaces at the beginning of the first line and
    // check if the line consists entirely of spaces or is empty
    bool onlySpaces = true;
    int n_spaces = 0;
    if (!selected_lines.isEmpty())
    {
        for (const auto& c : selected_lines[0])
        {
            if (c != ' ')
            {
                onlySpaces = false;
                break;
            }
            n_spaces++;
        }
    }
    // calculate indent to use for all selected lines based on the amount of
    // spaces needed to align the first line to the next_indent 4-block
    const int constant_indent = next_indent(indent, n_spaces);

    // if the cursor is in a word without a selection, show autocompletion menu
    // (skip this if we are un-indenting, meaning Shift+Tab has been pressed)
    if (indent && !preSelected && !onlySpaces)
    {
        handle_autocomplete();
        return;
    }

    // (un)indent all selected lines
    const int size = selected_lines.size();
    QString padding;
    if (indent)
    {
        padding = "";
        padding.fill(' ', constant_indent);
    }
    cursor.beginEditBlock();
    for (int i = 0; i < size; i++)
    {
        if (indent)
        {
            cursor.insertText(padding);
            end+=constant_indent;
        }
        else
        {
            QString current_line = selected_lines[i];
            // count how many spaces there really are so we don't cut off text
            int spaces = 0;
            while(spaces < constant_indent)
            {
                if (current_line[spaces] != ' ')
                {
                    break;
                }
                spaces++;
            }
            // un-indent line by removing spaces
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, spaces);
            cursor.removeSelectedText();
            end-=spaces;
        }
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
    }
    cursor.endEditBlock();
    // restore selection
    cursor.setPosition(start, QTextCursor::MoveAnchor);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
}

void python_code_editor::handle_return_pressed()
{
    QString current_line;
    {
        auto cursor = textCursor();
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        current_line = cursor.selectedText();
    }
    u32 num_spaces = 0;
    for (const auto& c : current_line)
    {
        if (c != ' ')
        {
            break;
        }
        num_spaces++;
    }

    auto trimmed = current_line.trimmed();
    if (!trimmed.isEmpty() && trimmed.at(trimmed.size() - 1) == ':')
    {
        num_spaces += 4;
    }

    num_spaces -= num_spaces % 4;
    insertPlainText("\n");
    for (u32 i = 0; i < num_spaces / 4; ++i)
    {
        insertPlainText("    ");
    }
}

void python_code_editor::handle_backspace_pressed(QKeyEvent* e)
{
    QString current_line;
    {
        auto cursor = textCursor();
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        current_line = cursor.selectedText();
    }

    if (!current_line.isEmpty() && current_line.trimmed().isEmpty() && current_line.size() % 4 == 0)
    {
        auto cursor = textCursor();
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 4);
        cursor.removeSelectedText();
        return;
    }

    QPlainTextEdit::keyPressEvent(e);
}

void python_code_editor::handle_delete_pressed(QKeyEvent* e)
{
    auto cursor = textCursor();
    if (cursor.positionInBlock() % 4 == 0)
    {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 4);
        if (cursor.selectedText() == "    ")
        {
            cursor.removeSelectedText();
            return;
        }
    }

    QPlainTextEdit::keyPressEvent(e);
}

void python_code_editor::perform_code_completion(std::tuple<std::string, std::string> completion)
{
    textCursor().insertText(QString::fromStdString(std::get<1>(completion)));
}

python_editor::python_editor(QWidget* parent)
    : content_widget("Python Editor", parent), python_context_subscriber(), m_editor_widget(new python_code_editor()), m_searchbar(new searchbar()), m_action_open_file(new QAction(this)),
      m_action_run(new QAction(this)), m_action_save(new QAction(this)), m_action_save_as(new QAction(this)), m_file_name("")
{
    ensurePolished();
    const int tab_stop = 4;

    QFontMetrics metrics(font());
    m_editor_widget->setTabStopWidth(tab_stop * metrics.width(" "));

    new python_syntax_highlighter(m_editor_widget->document());
    new python_syntax_highlighter(m_editor_widget->minimap()->document());

    m_content_layout->addWidget(m_editor_widget);
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
}

python_editor::~python_editor()
{
    g_settings.setValue("python_editor/code", m_editor_widget->toPlainText());
    g_settings.sync();
}

void python_editor::setup_toolbar(toolbar* toolbar)
{
    toolbar->addAction(m_action_open_file);
    toolbar->addAction(m_action_save);
    toolbar->addAction(m_action_save_as);
    toolbar->addAction(m_action_run);

    // DEBUG CODE
    QToolButton* button = new QToolButton(this);
    button->setText("Debug Toggle Minimap");
    button->setIcon(gui_utility::get_styled_svg_icon("all->#FFDD00", ":/icons/placeholder"));
    button->setToolTip("Debug Toggle Minimap");

    connect(button, &QToolButton::clicked, m_editor_widget, &code_editor::toggle_minimap);

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

    m_editor_widget->clear();
    std::string f((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    m_editor_widget->appendPlainText(QString::fromStdString(f));

    //file_manager::get_instance()->open_file(file_name);
}

void python_editor::save_file(const bool ask_path)
{
    QString title = "Save File";
    QString text  = "Python Scripts(*.py)";

    QString selected_file_name;

    if (ask_path || m_file_name.isEmpty())
    {
        selected_file_name = QFileDialog::getSaveFileName(nullptr, title, QDir::currentPath(), text, nullptr, QFileDialog::DontUseNativeDialog);
        if (selected_file_name.isEmpty())
        {
            return;
        }
    }
    else
    {
        selected_file_name = m_file_name;
    }

    std::ofstream out(selected_file_name.toStdString(), std::ios::out);

    if (!out.is_open())
    {
        return;
    }
    out << m_editor_widget->toPlainText().toStdString();
    out.close();

    // remember target file path
    m_file_name = selected_file_name;
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
    g_python_context->interpret_script(m_editor_widget->toPlainText());
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
