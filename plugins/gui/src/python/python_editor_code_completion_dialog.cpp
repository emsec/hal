#include "gui/python/python_editor_code_completion_dialog.h"

#include "hal_core/utilities/log.h"

#include <QAbstractItemView>
#include <QDebug>
#include <QHeaderView>
#include <QKeyEvent>
#include <QString>
#include <QStringList>
#include <QTableWidgetItem>
#include "hal_core/utilities/log.h"

namespace hal
{
    PythonEditorCodeCompletionDialog::PythonEditorCodeCompletionDialog(QWidget* parent, std::vector<std::tuple<std::string, std::string>> completions) : QDialog(parent), m_completions(completions)
    {
        //the parent has to be a layouter_view, needs a rework, for now its sufficient
        mTable = new QTableWidget(this);
        mTable->setParent(this);
        mTable->setShowGrid(false);
        mTable->verticalHeader()->setVisible(false);
        mTable->horizontalHeader()->setVisible(false);
        mTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        mTable->setColumnCount(1);
        mTable->setRowCount(completions.size());
        mTable->setStyleSheet("QTableView {background-color: black; color: white; selection-background-color: grey;};");
        mTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

        int counter = 0;
        for (const auto& tup : completions)
        {
            auto full_text = std::get<0>(tup);
            mTable->setItem(counter, 0, new QTableWidgetItem(QString::fromStdString(full_text)));
            counter++;
        }

        mTable->selectRow(0);
        mTable->resizeColumnsToContents();
        mTable->resizeRowsToContents();

        //check if the calculated minimum size is bigger than the predefined maximum size, if yes, set the minimum size = the predefined maximum size, otherwise set the minimum size
        //to the calculated size
        QSize calculatedSize = tableWidgetSize(mTable);
        if (calculatedSize.width() > 400)
            mTable->setMinimumWidth(400);
        else
            mTable->setMinimumWidth(calculatedSize.width());

        if (calculatedSize.height() > 600)
            mTable->setMinimumHeight(600);
        else
            mTable->setMinimumHeight(calculatedSize.height());

        //mTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        //mTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

        this->adjustSize();
        this->setFocusProxy(mTable);
        mTable->adjustSize();
        this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        this->setAttribute(Qt::WA_DeleteOnClose);

        //these lines need to be put after the lines above, dunno why
        this->setEnabled(true);
        this->setModal(true);
        this->setVisible(true);
    }

    //list is focused, so no need to implement down/up button, because the list automatically does the right thing for you
    void PythonEditorCodeCompletionDialog::keyPressEvent(QKeyEvent* event)
    {
        if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        {
            auto index = mTable->currentRow();
            Q_EMIT completionSelected(m_completions.at(index));
            this->close();
        }

        if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Left)
        {
            this->close();
        }
    }

    QSize PythonEditorCodeCompletionDialog::tableWidgetSize(QTableWidget* table)
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
