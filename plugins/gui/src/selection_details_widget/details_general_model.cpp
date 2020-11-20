#include "gui/selection_details_widget/details_general_model.h"
#include "gui/input_dialog/input_dialog.h"
#include <QTableView>
#include <QApplication>
#include <QClipboard>
#include <QMenu>

namespace hal {

//------------------------------------------------------------
    QVariant DetailsGeneralModelEntry::data(int iColumn) const
    {
        switch (iColumn) {
        case 0:
            return QString("%1:").arg(mLabel);
        case 1:
            return mValue;
        }
        return QVariant();
    }

    void DetailsGeneralModelEntry::setValue(const QString &v) const
    {
        if (!hasSetter()) return;
        mSetter(v.toStdString());
    }
//------------------------------------------------------------

    QString DetailsGeneralModel::moduleNameId(const Module *m)
    {
        if (!m) return QString("None");
        return QString("%1[%2]")
                .arg(QString::fromStdString(m->get_name()))
                .arg(m->get_id());
    }

    DetailsGeneralModel::DetailsGeneralModel(QObject* parent)
        : QAbstractTableModel(parent), mContextIndex(-1), mId(0), mGetParentModule(nullptr)
    {
        QTableView* parentView = static_cast<QTableView*>(parent);
        if (!parentView) return;
        connect(parentView, &QTableView::customContextMenuRequested, this, &DetailsGeneralModel::contextMenuRequested);
        connect(parentView, &QAbstractItemView::doubleClicked, this, &DetailsGeneralModel::handleDoubleClick);
        gSelectionRelay->registerSender(this, "SelectionDetailsWidget");
    }

    void DetailsGeneralModel::additionalInformation(Module* m)
    {
        mContent.append(DetailsGeneralModelEntry("Parent Module", moduleNameId(m->get_parent_module()), "get_parent_module"));

        int nGatesSumAll           = m->get_gates(nullptr, true).size();
        int nGatesDirectChild      = m->get_gates(nullptr, false).size();
        int nGatesGrandChild       = 0;

        for (auto module : m->get_submodules())
            nGatesGrandChild += module->get_gates(nullptr, true).size();

        QString numberGatesText = QString::number(nGatesSumAll);

        if (nGatesGrandChild > 0)
            numberGatesText += " (" + QString::number(nGatesDirectChild) + " direct members and " + QString::number(nGatesGrandChild) + " within submodules)";
        mContent.append(DetailsGeneralModelEntry("Gates", numberGatesText));
        uint nSubmodules = m->get_submodules(nullptr, true).size();
        mContent.append(DetailsGeneralModelEntry("Submodules", nSubmodules));
        uint nNets = m->get_internal_nets().size();
        mContent.append(DetailsGeneralModelEntry("Nets", nNets));

        // mContent :  Name=0, Type=1, Id=2, Grouping=3, Parent=4, Gates=5, Submodules=6, Nets=7
        mContent[0].assignSetter(std::bind(&Module::set_name,m,std::placeholders::_1));
        mContent[1].assignSetter(std::bind(&Module::set_type,m,std::placeholders::_1));
        mGetParentModule = std::bind(&Module::get_parent_module,m);
    }

    void DetailsGeneralModel::additionalInformation(Gate* g)
    {
        const Module* parentMod = nullptr;
        for (const Module* m : gNetlist->get_modules())
        {
            if (m->contains_gate(g))
            {
                parentMod = m;
                break;
            }
        }
        mContent.append(DetailsGeneralModelEntry("Module", moduleNameId(parentMod), "get_module"));
        mGetParentModule = std::bind(&Gate::get_module,g);
    }

    void DetailsGeneralModel::additionalInformation(Net* n)
    {
        mContent[0].assignSetter(std::bind(&Net::set_name,n,std::placeholders::_1));
    }


    QVariant DetailsGeneralModel::data(const QModelIndex &index, int role) const
    {
        if (role != Qt::DisplayRole) return QVariant();
        if (index.row() >= rowCount()) return QVariant();
        return mContent.at(index.row()).data(index.column());
    }

    int DetailsGeneralModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 2;
    }

    int DetailsGeneralModel::rowCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return mContent.size();
    }

    QString DetailsGeneralModel::pythonCommand(const QString& pyGetter) const
    {
        if (pyGetter.isEmpty()) return QString();
        return mPythonBase + pyGetter + "()";
    }

    void DetailsGeneralModel::extractRawTriggered() const
    {
        if (mContextIndex < 0 || mContextIndex >= mContent.size()) return;
        const DetailsGeneralModelEntry& dgme = mContent.at(mContextIndex);
        QApplication::clipboard()->setText(dgme.textValue());
    }

    void DetailsGeneralModel::extractPythonTriggered() const
    {
        if (mContextIndex < 0 || mContextIndex >= mContent.size()) return;
        const DetailsGeneralModelEntry& dgme = mContent.at(mContextIndex);
        QApplication::clipboard()->setText(pythonCommand(dgme.pythonGetter()));
    }

    void DetailsGeneralModel::editValueTriggered()
    {
        if (mContextIndex < 0 || mContextIndex >= mContent.size()) return;
        const DetailsGeneralModelEntry& dgme = mContent.at(mContextIndex);
        InputDialog ipd("Change "+dgme.lcLabel(), "New "+dgme.lcLabel(), dgme.textValue());
        if (ipd.exec() == QDialog::Accepted)
        {
            mContent.at(mContextIndex).setValue(ipd.textValue());
            Q_EMIT requireUpdate(mId);
        }
    }

    void DetailsGeneralModel::handleDoubleClick(const QModelIndex &inx)
    {
        if (!inx.isValid()) return;
        if (mContent.at(inx.row()).lcLabel().endsWith("module"))
        {
            // request: navigate to parent module
            if (!mGetParentModule) return;
            Module* parMod = mGetParentModule();
            if (!parMod) return;
            gSelectionRelay->clear();
            gSelectionRelay->mSelectedModules.insert(parMod->get_id());
            gSelectionRelay->relaySelectionChanged(this);
        }
    }


    void DetailsGeneralModel::contextMenuRequested(const QPoint& pos)
    {
        QTableView* tv = static_cast<QTableView*>(parent());
        if (!tv) return;
        QModelIndex inx = tv->indexAt(pos);
        if (!inx.isValid()) return;
        mContextIndex = inx.row();
        const DetailsGeneralModelEntry& dgme = mContent.at(mContextIndex);
        if (!dgme.hasSetter() && dgme.pythonGetter().isEmpty()) return;
        QMenu* contextMenu = new QMenu(tv);
        QAction* action;
        if (dgme.hasSetter())
        {
            action = contextMenu->addAction("Change " + dgme.lcLabel());
            connect(action, &QAction::triggered, this, &DetailsGeneralModel::editValueTriggered);
        }
        if (!dgme.pythonGetter().isEmpty())
        {
            action = contextMenu->addAction("Extract raw " + dgme.lcLabel() + " (copy to clipboard)");
            connect(action, &QAction::triggered, this, &DetailsGeneralModel::extractRawTriggered);
            action = contextMenu->addAction(QIcon(":/icons/python"),"Extract " + dgme.lcLabel() + " as python code (copy to clipboard)");
            connect(action, &QAction::triggered, this, &DetailsGeneralModel::extractPythonTriggered);
        }
        contextMenu->exec(tv->viewport()->mapToGlobal(pos));
    }

}
