#include "gui/vhdl_editor/vhdl_editor.h"

#include "hal_core/netlist/hdl_writer/hdl_writer_manager.h"

#include "gui/code_editor/syntax_highlighter/vhdl_syntax_highlighter.h"
#include "gui/gui_globals.h"
#include "gui/searchbar/searchbar.h"

#include <QShortcut>
#include <QVBoxLayout>

namespace hal
{
    VhdlEditor::VhdlEditor() : ContentWidget("Source"), mCodeEditor(new CodeEditor()), mSearchbar(new Searchbar())
    {
        connect(mSearchbar, &Searchbar::textEdited, mCodeEditor, &CodeEditor::search);

        mCodeEditor->setReadOnly(true);
        mSearchbar->hide();

        new VhdlSyntaxHighlighter(mCodeEditor->document());
        new VhdlSyntaxHighlighter(mCodeEditor->minimap()->document());

        std::stringstream stream;
        hdl_writer_manager::write(gNetlist, ".vhd", stream);
        QString string = QString::fromStdString(stream.str());
        mCodeEditor->setPlainText(string);

        mContentLayout->addWidget(mCodeEditor);
        mContentLayout->addWidget(mSearchbar);
    }

    void VhdlEditor::setupToolbar(Toolbar* Toolbar){Q_UNUSED(Toolbar)}

    QList<QShortcut*> VhdlEditor::createShortcuts()
    {
        QShortcut* search_shortcut = new QShortcut(QKeySequence("Ctrl+f"), this);
        connect(search_shortcut, &QShortcut::activated, this, &VhdlEditor::toggleSearchbar);

        QList<QShortcut*> list;
        list.append(search_shortcut);

        return list;
    }

    void VhdlEditor::toggleSearchbar()
    {
        if (mSearchbar->isHidden())
            mSearchbar->show();
        else
            mSearchbar->hide();
    }
}
