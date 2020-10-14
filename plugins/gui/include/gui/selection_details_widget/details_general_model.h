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
        DataContainer* mItem;
        QString mLabel;
        QVariant mValue;
        QString mPythonGetter;
        std::function<void(const std::string&)> mSetter;

    public:
        DetailsGeneralModelEntry()
            : mItem(nullptr), mSetter(nullptr) {;}
        DetailsGeneralModelEntry(DataContainer* item_,
                                 const QString& label_,
                                 const QVariant& value_,
                                 const QString& python_ = QString())
            : mItem(item_), mLabel(label_), mValue(value_), mPythonGetter(python_), mSetter(nullptr) {;}
        QString pythonGetter() const { return mPythonGetter; };
        void assignSetter(const std::function<void(const std::string&)>& setter_) { mSetter = setter_; }
        QVariant data(int iColumn) const;
        bool hasSetter() const { return mSetter != nullptr; }
        void setValue(const QString& v) const;
        QString lcLabel() const { return mLabel.toLower(); }
        QString textValue() const { return mValue.toString(); }
    };

    template <typename T> class DetailsGeneralCommonInfo
    {
    public:
        enum itemType_t {TypeUndefined, TypeModule, TypeGate, TypeNet};
    private:
        itemType_t mItemType;
        QString mItemName;
        QString mTypeName;
        u32 mId;
        Grouping* mGrouping;
        QString mPythonBase;

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
            if (g_netlist->is_global_input_net(n))
                mTypeName = "Input";
            if (g_netlist->is_global_output_net(n))
                mTypeName = "Output";
        }

        void setPythonBase(Module* m)
        {
            mPythonBase = QString("netlist.get_module_by_id(%1).").arg(m->get_id());
        }

        void setPythonBase(Gate* g)
        {
            mPythonBase = QString("netlist.get_gate_by_id(%1).").arg(g->get_id());
        }

        void setPythonBase(Net* n)
        {
            mPythonBase = QString("netlist.get_net_by_id(%1).").arg(n->get_id());
        }

    public:
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

        QString py(const QString& pyGetter) const
        {
            if (pyGetter.isEmpty()) return QString();
            return mPythonBase + pyGetter;
        }

        DetailsGeneralCommonInfo(T* item)
        {
            mItemName =  QString::fromStdString(item->get_name());
            mId       =  item->get_id();
            mGrouping =  item->get_grouping();
            setTypeName(item);
            setPythonBase(item);
        }
    };

    class DetailsGeneralModel : public QAbstractTableModel
    {
        Q_OBJECT
        QList<DetailsGeneralModelEntry> mContent;
        int mContextIndex;
        u32 mId;

        void additionalInformation(Module* m);
        void additionalInformation(Gate* g);
        void additionalInformation(Net* n);

        static QString moduleNameId(const Module* m);

    public Q_SLOTS:
        void contextMenuRequested(const QPoint& pos);
        void editValueTriggered();

    Q_SIGNALS:
        void requireUpdate(u32 id);

    public:
        template <typename T> void setContent(T *item)
        {
            mContent.clear();
            DetailsGeneralCommonInfo<T> dgi(item);
            mId = dgi.id();
            DetailsGeneralModelEntry dgmeName(item,"Name",     dgi.name(), dgi.py("get_name"));
            if (typeid(T) != typeid(Gate))
                dgmeName.assignSetter(std::bind(&T::set_name,item,std::placeholders::_1));
            mContent.append(dgmeName);
            mContent.append(DetailsGeneralModelEntry(item,"Type",     dgi.typeName(), dgi.py("get_type")));
            mContent.append(DetailsGeneralModelEntry(item,"ID",       dgi.id(), dgi.py("get_id")));
            mContent.append(DetailsGeneralModelEntry(item,"Grouping", dgi.grouping(), dgi.py("get_grouping")));

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
                n = 7;
            else if (typeid(T) == typeid(Gate))
                n = 4;
            else if (typeid(T) == typeid(Net))
                n = 3;
            for (int i=0; i<n; i++)
                mContent.append(DetailsGeneralModelEntry());
        }

        DetailsGeneralModel(QObject* parent = nullptr);

        QVariant data(const QModelIndex &index, int role) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    };
}
