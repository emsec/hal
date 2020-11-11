#pragma once

#include <QAbstractTableModel>
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/grouping.h"
#include "gui/gui_globals.h"
#include <QStringList>
#include <QPair>

namespace hal {

    class DetailsGeneralModelEntry
    {
        QString mLabel;
        QVariant mValue;
        QString mPythonGetter;
        std::function<void(const std::string&)> mSetter;

    public:
        DetailsGeneralModelEntry()
            : mSetter(nullptr) {;}
        DetailsGeneralModelEntry(const QString& label_,
                                 const QVariant& value_,
                                 const QString& python_ = QString())
            : mLabel(label_), mValue(value_), mPythonGetter(python_), mSetter(nullptr)
        {;}
        QVariant data(int iColumn) const;
        bool hasSetter() const { return mSetter != nullptr; }
        void setValue(const QString& v) const;
        void assignSetter(const std::function<void(const std::string&)>& setter_) { mSetter = setter_; }
        QString lcLabel() const { return mLabel.toLower(); }
        QString textValue() const { return mValue.toString(); }
        QString pythonGetter() const { return mPythonGetter; }
    };

    template <typename T> class DetailsGeneralCommonInfo
    {
    private:
        QString mItemName;
        QString mTypeName;
        u32 mId;
        Grouping* mGrouping;

        void setTypeName(Module* m)
        {
            mTypeName = QString::fromStdString(m->get_type());
        }

        void setTypeName(Gate *g)
        {
            const GateType* gt = g->get_type();
            if (gt) mTypeName = QString::fromStdString(gt->get_name());
        }

        void setTypeName(Net *n)
        {
            mTypeName = "Internal";
            if (gNetlist->is_global_input_net(n))
                mTypeName = "Input";
            if (gNetlist->is_global_output_net(n))
                mTypeName = "Output";
        }

    public:
        QString getPythonBase(Module* m)
        {
            return QString("netlist.get_module_by_id(%1).").arg(m->get_id());
        }

        QString getPythonBase(Gate* g)
        {
            return QString("netlist.get_gate_by_id(%1).").arg(g->get_id());
        }

        QString getPythonBase(Net* n)
        {
            return QString("netlist.get_net_by_id(%1).").arg(n->get_id());
        }

        QString name() const
        {
            if (mItemName.isEmpty()) return "None";
            return mItemName;
        }

        QString grouping() const
        {
            if (!mGrouping) return "None";
            return QString::fromStdString(mGrouping->get_name());
        }

        QString typeName() const
        {
            if (mTypeName.isEmpty()) return "None";
            return mTypeName;
        }

        u32 id() const { return mId; }

        DetailsGeneralCommonInfo(T* item)
        {
            mItemName =  QString::fromStdString(item->get_name());
            mId       =  item->get_id();
            mGrouping =  item->get_grouping();
            setTypeName(item);
        }
    };

    class DetailsGeneralModel : public QAbstractTableModel
    {
        Q_OBJECT
        QList<DetailsGeneralModelEntry> mContent;
        int mContextIndex;
        u32 mId;
        QString mPythonBase;
        std::function<Module*(void)> mGetParentModule;

        void additionalInformation(Module* m);
        void additionalInformation(Gate* g);
        void additionalInformation(Net* n);

        QString pythonCommand(const QString& pyGetter) const;
        static QString moduleNameId(const Module* m);

    public Q_SLOTS:
        void contextMenuRequested(const QPoint& pos);
        void handleDoubleClick(const QModelIndex &inx);
        void editValueTriggered();
        void extractRawTriggered() const;
        void extractPythonTriggered() const;

    Q_SIGNALS:
        void requireUpdate(u32 id);

    public:
        template <typename T> void setContent(T *item)
        {
            mContent.clear();
            DetailsGeneralCommonInfo<T> dgi(item);
            mId         = dgi.id();
            mPythonBase = dgi.getPythonBase(item);
            mContent.append(DetailsGeneralModelEntry("Name",     dgi.name(),     "get_name"));
            mContent.append(DetailsGeneralModelEntry("Type",     dgi.typeName(), "get_type"));
            mContent.append(DetailsGeneralModelEntry("ID",       mId,            "get_id"));
            mContent.append(DetailsGeneralModelEntry("Grouping", dgi.grouping(), "get_grouping"));

            additionalInformation(item);

            int n = columnCount() - 1;
            QModelIndex inx0 = index(0,0);
            QModelIndex inx1 = index(n,1);
            Q_EMIT dataChanged(inx0,inx1);

        }

        template <typename T> void setDummyContent()
        {
            mContextIndex = -1;
            mId = 0;
            mContent.clear();
            int n = 0;
            if (typeid(T) == typeid(Module))
                n = 8;
            else if (typeid(T) == typeid(Gate))
                n = 5;
            else if (typeid(T) == typeid(Net))
                n = 4;
            for (int i=0; i<n; i++)
                mContent.append(DetailsGeneralModelEntry());
        }

        DetailsGeneralModel(QObject* parent = nullptr);

        QVariant data(const QModelIndex &index, int role) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    };
}
