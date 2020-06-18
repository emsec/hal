#include "python/python_editor_code_completion_dialog.h"

#include "core/log.h"

#include <QAbstractItemView>
#include <QDebug>
#include <QHeaderView>
#include <QKeyEvent>
#include <QString>
#include <QStringList>
#include <QTableWidgetItem>
#include <core/log.h>
namespace hal{
python_editor_code_completion_dialog::python_editor_code_completion_dialog(QWidget* parent, std::vector<std::tuple<std::string, std::string>> completions) : QDialog(parent), m_completions(completions)
{
    //the parent has to be a layouter_view, needs a rework, for now its sufficient
    m_table = new QTableWidget(this);
    m_table->setParent(this);
    m_table->setShowGrid(false);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setColumnCount(1);
    m_table->setRowCount(completions.size());
    m_table->setStyleSheet("QTableView {background-color: black; color: white; selection-background-color: grey;};");
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    int counter = 0;
    for (const auto& tup : completions)
    {
        auto full_text = std::get<0>(tup);
        m_table->setItem(counter, 0, new QTableWidgetItem(QString::fromStdString(full_text)));
        counter++;
    }

    m_table->selectRow(0);
    m_table->resizeColumnsToContents();
    m_table->resizeRowsToContents();

    //check if the calculated minimum size is bigger than the predefined maximum size, if yes, set the minimum size = the predefined maximum size, otherwise set the minimum size
    //to the calculated size
    QSize calculatedSize = table_widget_size(m_table);
    if (calculatedSize.width() > 400)
        m_table->setMinimumWidth(400);
    else
        m_table->setMinimumWidth(calculatedSize.width());

    if (calculatedSize.height() > 600)
        m_table->setMinimumHeight(600);
    else
        m_table->setMinimumHeight(calculatedSize.height());

    //m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    //m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    this->adjustSize();
    this->setFocusProxy(m_table);
    m_table->adjustSize();
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_DeleteOnClose);

    //these lines need to be put after the lines above, dunno why
    this->setEnabled(true);
    this->setModal(true);
    this->setVisible(true);
}

//list is focused, so no need to implement down/up button, because the list automatically does the right thing for you
void python_editor_code_completion_dialog::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        auto index = m_table->currentRow();
        Q_EMIT completionSelected(m_completions.at(index));
        this->close();
    }

    if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Left)
    {
        this->close();
    }
}

QSize python_editor_code_completion_dialog::table_widget_size(QTableWidget* table)
{
    int width = 4;
    for (int i = 0; i < table->columnCount(); i++)
        width += table->columnWidth(i);

    int height = table->horizontalHeader()->height() + 8;
    for (int i = 0; i < table->rowCount(); i++)
        height += table->rowHeight(i);

    return QSize(width, height);
}
}
