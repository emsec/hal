#include "vhdl_editor/vhdl_editor.h"

#include "netlist/hdl_writer/hdl_writer_manager.h"

#include "code_editor/syntax_highlighter/vhdl_syntax_highlighter.h"
#include "gui_globals.h"
#include "searchbar/searchbar.h"

#include <QShortcut>
#include <QVBoxLayout>

namespace hal
{
    VhdlEditor::VhdlEditor() : ContentWidget("Source"), m_code_editor(new CodeEditor()), m_searchbar(new Searchbar())
    {
        connect(m_searchbar, &Searchbar::text_edited, m_code_editor, &CodeEditor::search);

        m_code_editor->setReadOnly(true);
        m_searchbar->hide();

        new VhdlSyntaxHighlighter(m_code_editor->document());
        new VhdlSyntaxHighlighter(m_code_editor->minimap()->document());

        std::stringstream stream;
        hdl_writer_manager::write(g_netlist, ".vhd", stream);
        QString string = QString::fromStdString(stream.str());
        m_code_editor->setPlainText(string);

        m_content_layout->addWidget(m_code_editor);
        m_content_layout->addWidget(m_searchbar);
    }

    void VhdlEditor::setup_toolbar(Toolbar* Toolbar){Q_UNUSED(Toolbar)}

    QList<QShortcut*> VhdlEditor::create_shortcuts()
    {
        QShortcut* search_shortcut = new QShortcut(QKeySequence("Ctrl+f"), this);
        connect(search_shortcut, &QShortcut::activated, this, &VhdlEditor::toggle_searchbar);

        QList<QShortcut*> list;
        list.append(search_shortcut);

        return list;
    }

    void VhdlEditor::toggle_searchbar()
    {
        if (m_searchbar->isHidden())
            m_searchbar->show();
        else
            m_searchbar->hide();
    }
}
