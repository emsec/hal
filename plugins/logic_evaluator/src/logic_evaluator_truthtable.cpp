#include "logic_evaluator/logic_evaluator_truthtable.h"
#include "hal_core/netlist/net.h"

#include <QGridLayout>
#include <QTableView>
#include <QMenuBar>
#include <QActionGroup>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QLabel>

namespace hal {
    LogicEvaluatorTruthtableColumn::LogicEvaluatorTruthtableColumn(int nrows, QList<int> values)
        : mRows(nrows)
    {
        mArray = new int[mRows];
        for (int i=0; i<mRows; i++)
        {
            mArray[i] = values.at(i);
        }
    }

    bool LogicEvaluatorTruthtableColumn::lessThan(const LogicEvaluatorTruthtableColumn& other, const QList<int> &sortRows) const
    {
        for (int irow : sortRows)
        {
            if (mArray[irow] < other.mArray[irow]) return true;
            if (mArray[irow] > other.mArray[irow]) return false;
        }
        return false; // equal
    }

    LogicEvaluatorTruthtableColumn::~LogicEvaluatorTruthtableColumn()
    {
        delete [] mArray;
    }

    int LogicEvaluatorTruthtableColumn::data(int irow) const
    {
        return mArray[irow];
    }

    //-----------------------------
    LogicEvaluatorTruthtableModel::LogicEvaluatorTruthtableModel(const QList<const Net *> &inpList, const QList<const Net *> &outList, QObject* parent)
        : mDisplayFormat(MAXFORMAT), // will be set by setDisplayFormat
          mInputList(inpList), mOutputList(outList), mInputSize(inpList.size()), mOutputSize(outList.size())
    {;}

    LogicEvaluatorTruthtableModel::~LogicEvaluatorTruthtableModel()
    {
        for (LogicEvaluatorTruthtableColumn* letc : mColumnList)
            delete letc;
    }

    void LogicEvaluatorTruthtableModel::addColumn(LogicEvaluatorTruthtableColumn* letc)
    {
        mColumnList.append(letc);
    }

    void LogicEvaluatorTruthtableModel::sortModelRow(int irow)
    {
        QList<int> sortRows;
        sortRows.append(irow);
        std::sort(mColumnList.begin(),mColumnList.end(),[sortRows](const LogicEvaluatorTruthtableColumn* a, const LogicEvaluatorTruthtableColumn* b){return a->lessThan(*b,sortRows); } );
        Q_EMIT dataChanged(index(0,0),index(rowCount()-1,columnCount()-1));
    }

    void LogicEvaluatorTruthtableModel::sortModelRows(const QList<int>& sortRows)
    {
        if (sortRows.isEmpty()) return;
        std::sort(mColumnList.begin(),mColumnList.end(),[sortRows](const LogicEvaluatorTruthtableColumn* a, const LogicEvaluatorTruthtableColumn* b){return a->lessThan(*b,sortRows); } );
        Q_EMIT dataChanged(index(0,0),index(rowCount()-1,columnCount()-1));
    }

    QVariant LogicEvaluatorTruthtableModel::data(const QModelIndex& index, int role) const
    {
        int val = mColumnList.at(index.column())->data(index.row());

        switch (role)
        {
        case Qt::DisplayRole:
            switch (mDisplayFormat) {
            case ZeroOne:
                return val;
            case LowHigh:
                return val ? "H" : "L";
            case BlueRed:
                return QChar(0x2588);
            }
            break;
        case Qt::ForegroundRole:
            if (mDisplayFormat == BlueRed)
                return val ? QColor("#FF0000") : QColor("#00A0FF");
            break;
        case Qt::BackgroundRole:
            if (index.row() >= mInputSize)
                return QColor("#080930");
            return QColor("302A12");
            break;
        case Qt::TextAlignmentRole:
            return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
        default:
            break;
        }
        return QVariant();
    }

    QVariant LogicEvaluatorTruthtableModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role!=Qt::DisplayRole) return QAbstractTableModel::headerData(section, orientation, role);
        if (orientation == Qt::Horizontal)
            return QString::number(section, 16);
        if (section < mInputSize)
            return QString::fromStdString(mInputList.at(section)->get_name());
        return QString::fromStdString(mOutputList.at(section-mInputSize)->get_name());
    }

    int LogicEvaluatorTruthtableModel::rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return mInputSize + mOutputSize;
    }

    int LogicEvaluatorTruthtableModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return (1 << mInputList.size());
    }

    //--------------------------------
    LogicEvaluatorTruthtableSort::LogicEvaluatorTruthtableSort(QList<const Net*>& nets, QWidget* parent)
        : QDialog(parent), mNets(nets)
    {
        QVBoxLayout* layout = new QVBoxLayout(this);
        const char* ordinal[] = { "st", "nd", "th"};
        for (int i=0; i<5; i++)
        {
            if (i) layout->addStretch();
            layout->addWidget(new QLabel(QString("%1%2 sort key").arg(i+1).arg(i<3?ordinal[i]:ordinal[2]),this));
            mSortKey[i] = new QComboBox(this);
            mSortKey[i]->addItem("--not used--", (const void*) nullptr);
            for (const Net* n : mNets)
                mSortKey[i]->addItem(QString::fromStdString(n->get_name()),(const void*)n);
            layout->addWidget(mSortKey[i]);
        }
        QDialogButtonBox* bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(bbox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(bbox, &QDialogButtonBox::rejected, this , &QDialog::reject);
        bbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        layout->addWidget(bbox);
    }

    QList<int> LogicEvaluatorTruthtableSort::sortOrder() const
    {
         QList<int> retval;
         QSet<const Net*> selected;
         for (int i=0; i<5; i++)
         {
             int inx = mSortKey[i]->currentIndex();
             if (!inx) break;
             --inx; // skip not used entry
             const Net* n = mNets.at(inx);
             if (!selected.contains(n))
             {
                 retval.append(inx);
                 selected.insert(n);
             }
         }
         return retval;
    }

    //--------------------------------
    LogicEvaluatorTruthtable::LogicEvaluatorTruthtable(LogicEvaluatorTruthtableModel* model, QWidget* parent)
        : mModel(model), mColumnDubbleClicked(-1)
    {
        QGridLayout* layout = new QGridLayout(this);
        QMenuBar* menuBar = new QMenuBar(this);
        QMenu* displForm = menuBar->addMenu("Format");

        const char* displayFormatLabel[] = {"0 / 1", "L / H", "blue / red"};
        QActionGroup* actGroup = new QActionGroup(this);
        for (int i=0; i< LogicEvaluatorTruthtableModel::MAXFORMAT; i++)
        {
            mActionDisplayFormat[i] = displForm->addAction(displayFormatLabel[i]);
            mActionDisplayFormat[i]->setCheckable(true);
            actGroup->addAction(mActionDisplayFormat[i]);
        }
        connect(actGroup, &QActionGroup::triggered, this, &LogicEvaluatorTruthtable::handleDisplayFormatChanged);
        mActionDisplayFormat[LogicEvaluatorTruthtableModel::ZeroOne]->setChecked(true);
        handleDisplayFormatChanged(mActionDisplayFormat[LogicEvaluatorTruthtableModel::ZeroOne]);

        QAction* actSort = menuBar->addAction("Sort");
        connect(actSort, &QAction::triggered, this, &LogicEvaluatorTruthtable::handleSortTriggered);

        QTableView* view = new QTableView(this);
        view->setModel(mModel);
        for (int icol=0; icol<mModel->columnCount(); icol++)
            view->setColumnWidth(icol, 32);
        connect(view->verticalHeader(), &QHeaderView::sectionClicked, mModel, &LogicEvaluatorTruthtableModel::sortModelRow);
        connect(view->horizontalHeader(), &QHeaderView::sectionDoubleClicked, this, &LogicEvaluatorTruthtable::handleColumnDubbleClicked);
        layout->addWidget(view);
        layout->setMenuBar(menuBar);
    }

    void LogicEvaluatorTruthtableModel::setDisplayFormat(DisplayFormat df)
    {
        if (df == mDisplayFormat) return;
        mDisplayFormat = df;
        Q_EMIT dataChanged(index(0,0),index(rowCount()-1,columnCount()-1));
    }

    void LogicEvaluatorTruthtable::handleSortTriggered()
    {
        QList<const Net*> nets = mModel->getNets();
        LogicEvaluatorTruthtableSort lets(nets,this);
        if (lets.exec() == QDialog::Accepted)
        {
            mModel->sortModelRows(lets.sortOrder());
        }
    }

    void LogicEvaluatorTruthtable::handleColumnDubbleClicked(int icol)
    {
        mColumnDubbleClicked = icol;
        accept();
    }

    QMap<const Net*, int> LogicEvaluatorTruthtableModel::selectedColumn(int icol) const
    {
        QMap<const Net*,int> retval;
        if (icol < 0 || icol >= mColumnList.size()) return retval;
        LogicEvaluatorTruthtableColumn* letc = mColumnList.at(icol);
        int irow = 0;
        for (const Net* n : mInputList)
            retval[n] = letc->data(irow++);
        return retval;
    }

    QMap<const Net*, int> LogicEvaluatorTruthtable::selectedColumn() const
    {
        return mModel->selectedColumn(mColumnDubbleClicked);
    }

    void LogicEvaluatorTruthtable::handleDisplayFormatChanged(QAction* act)
    {
        for (int i=0; i<LogicEvaluatorTruthtableModel::MAXFORMAT; i++)
            if (mActionDisplayFormat[i] == act)
            {
                mModel->setDisplayFormat((LogicEvaluatorTruthtableModel::DisplayFormat)i);
                break;
            }
    }
}
