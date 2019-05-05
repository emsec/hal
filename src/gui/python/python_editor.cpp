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
#include <QToolButton>
#include <QVBoxLayout>

#include <fstream>

void python_code_editor::keyPressEvent(QKeyEvent* e)
{
    if (textCursor().hasSelection())
    {
        QPlainTextEdit::keyPressEvent(e);
        return;
    }

    if (e->key() == Qt::Key_Tab)
    {
        handle_tab_key_pressed();
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

void python_code_editor::handle_tab_key_pressed()
{
    QString current_line;
    {
        auto cursor = textCursor();
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        current_line = cursor.selectedText();
    }
    bool onlySpaces = true;
    for (const auto& c : current_line)
    {
        if (c != ' ')
        {
            onlySpaces = false;
            break;
        }
    }
    if (onlySpaces)
    {
        auto current_len = current_line.size();
        if (current_len % 4 != 0)
        {
            for (int i = 0; i < 4 - (current_len % 4); ++i)
            {
                insertPlainText(" ");
            }
        }
        else
        {
            insertPlainText("    ");
        }
    }
    else
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
      m_action_run(new QAction(this)), m_action_save(new QAction(this)), m_file_name("")
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
    m_action_run->setIcon(gui_utility::get_styled_svg_icon(m_run_icon_style, m_run_icon_path));

    m_action_open_file->setShortcut(QKeySequence("Ctrl+Shift+O"));
    m_action_save->setShortcut(QKeySequence("Shift+Ctrl+S"));
    m_action_run->setShortcut(QKeySequence("Ctrl+R"));

    m_action_open_file->setText("Open Script '" + m_action_open_file->shortcut().toString(QKeySequence::NativeText) + "'");
    m_action_save->setText("Save '" + m_action_save->shortcut().toString(QKeySequence::NativeText) + "'");
    m_action_run->setText("Execute Script '" + m_action_run->shortcut().toString(QKeySequence::NativeText) + "'");

    connect(m_action_open_file, &QAction::triggered, this, &python_editor::handle_action_open_file);
    connect(m_action_save, &QAction::triggered, this, &python_editor::handle_action_save_file);
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

    m_file_name = QFileDialog::getOpenFileName(nullptr, title, QDir::currentPath(), text, nullptr, QFileDialog::DontUseNativeDialog);

    if (m_file_name.isEmpty())
    {
        return;
    }

    std::ifstream file(m_file_name.toStdString(), std::ios::in);

    if (!file.is_open())
    {
        return;
    }

    m_editor_widget->clear();
    std::string f((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    m_editor_widget->appendPlainText(QString::fromStdString(f));

    //file_manager::get_instance()->open_file(file_name);
}

void python_editor::handle_action_save_file()
{
    QString title = "Save File";
    QString text  = "Python Scripts(*.py)";

    if (m_file_name.isEmpty())
    {
        m_file_name = QFileDialog::getSaveFileName(nullptr, title, QDir::currentPath(), text, nullptr, QFileDialog::DontUseNativeDialog);
    }
    std::ofstream out(m_file_name.toStdString(), std::ios::out);

    if (!out.is_open())
    {
        return;
    }
    out << m_editor_widget->toPlainText().toStdString();
    out.close();
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
