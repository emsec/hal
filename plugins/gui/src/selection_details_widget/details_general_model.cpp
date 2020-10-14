#include "gui/selection_details_widget/details_general_model.h"
#include "gui/input_dialog/input_dialog.h"
#include <QTableView>
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
        : QAbstractTableModel(parent), mContextIndex(-1)
    {;}

    void DetailsGeneralModel::additionalInformation(Module* m)
    {
        mContent.append(DetailsGeneralModelEntry(m,"Parent module:",
                                                 moduleNameId(m->get_parent_module())));

        int nGatesSumAll           = m->get_gates(nullptr, true).size();
        int nGatesDirectChild      = m->get_gates(nullptr, false).size();
        int nGatesGrandChild       = 0;

        for (auto module : m->get_submodules())
            nGatesGrandChild += module->get_gates(nullptr, true).size();

        QString numberGatesText = QString::number(nGatesSumAll);

        if (nGatesGrandChild > 0)
            numberGatesText += " in total, " + QString::number(nGatesDirectChild) + " as direct members and " + QString::number(nGatesGrandChild) + " in submodules";
        mContent.append(DetailsGeneralModelEntry(m,"Gates:", numberGatesText));
        uint nSubmodules = m->get_submodules(nullptr, true).size();
        mContent.append(DetailsGeneralModelEntry(m,"Submodules:", nSubmodules));
        uint nNets = m->get_internal_nets().size();
        mContent.append(DetailsGeneralModelEntry(m,"Nets:", nNets));
    }

    void DetailsGeneralModel::additionalInformation(Gate* g)
    {
        const Module* parentMod = nullptr;
        for (const Module* m : g_netlist->get_modules())
        {
            if (m->contains_gate(g))
            {
                parentMod = m;
                break;
            }
        }
        mContent.append(DetailsGeneralModelEntry(g,"Module:", moduleNameId(parentMod)));
    }

    void DetailsGeneralModel::additionalInformation(Net* n)
    {
        Q_UNUSED(n);
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

    void DetailsGeneralModel::editValueTriggered()
    {
        if (mContextIndex < 0 || mContextIndex >= mContent.size()) return;
        const DetailsGeneralModelEntry& dgme = mContent.at(mContextIndex);
        InputDialog ipd("Change "+dgme.lcLabel(), "New "+dgme.lcLabel(), dgme.textValue());
        if (ipd.exec() == QDialog::Accepted)
        {
            mContent.at(mContextIndex).setValue(ipd.text_value());
            Q_EMIT requireUpdate(mId);
        }
    }

    void DetailsGeneralModel::contextMenuRequested(const QPoint& pos)
    {
        QTableView* tv = static_cast<QTableView*>(parent());
        if (!tv) return;
        QModelIndex inx = tv->indexAt(pos);
        if (!inx.isValid()) return;
        mContextIndex = inx.row();
        QMenu* contextMenu = new QMenu(tv);
        const DetailsGeneralModelEntry& dgme = mContent.at(mContextIndex);
        if (dgme.hasSetter())
        {
            QAction* actionSetValue = contextMenu->addAction("Change "+dgme.lcLabel());
            connect(actionSetValue, &QAction::triggered, this, &DetailsGeneralModel::editValueTriggered);
        }
        contextMenu->addAction(QString("action <%1>").arg(data(inx,Qt::DisplayRole).toString()));
        QString py = dgme.pythonGetter();
        if (!py.isEmpty()) contextMenu->addAction(py);
        contextMenu->exec(tv->viewport()->mapToGlobal(pos));
    }

}
