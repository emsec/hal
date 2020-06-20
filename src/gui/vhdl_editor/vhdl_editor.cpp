#include "vhdl_editor/vhdl_editor.h"

#include "netlist/hdl_writer/hdl_writer_vhdl.h"

#include "code_editor/syntax_highlighter/vhdl_syntax_highlighter.h"
#include "gui_globals.h"
#include "searchbar/searchbar.h"

#include <QShortcut>
#include <QVBoxLayout>

namespace hal
{
    vhdl_editor::vhdl_editor() : ContentWidget("Source"), m_code_editor(new CodeEditor()), m_searchbar(new searchbar())
    {
        connect(m_searchbar, &searchbar::text_edited, m_code_editor, &CodeEditor::search);

        m_code_editor->setReadOnly(true);
        m_searchbar->hide();

        new VhdlSyntaxHighlighter(m_code_editor->document());
        new VhdlSyntaxHighlighter(m_code_editor->minimap()->document());

        std::stringstream stream;
        HDLWriterVHDL writer(stream);
        writer.write(g_netlist);
        QString string = QString::fromStdString(stream.str());
        m_code_editor->setPlainText(string);

        m_content_layout->addWidget(m_code_editor);
        m_content_layout->addWidget(m_searchbar);
    }

    void vhdl_editor::setup_toolbar(toolbar* toolbar){Q_UNUSED(toolbar)}

    QList<QShortcut*> vhdl_editor::create_shortcuts()
    {
        QShortcut* search_shortcut = new QShortcut(QKeySequence("Ctrl+f"), this);
        connect(search_shortcut, &QShortcut::activated, this, &vhdl_editor::toggle_searchbar);

        QList<QShortcut*> list;
        list.append(search_shortcut);

        return list;
    }

    void vhdl_editor::toggle_searchbar()
    {
        if (m_searchbar->isHidden())
            m_searchbar->show();
        else
            m_searchbar->hide();
    }
}
