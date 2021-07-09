//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.


#pragma once

#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/grouping.h"

#include "gui/gui_globals.h"
#include "gui/user_action/user_action_object.h"

#include <QAbstractTableModel>
#include <QStringList>
#include <QPair>

namespace hal {

    /**
     * @ingroup utility_widgets-selection_details
     * @brief Single entry in the GeneralDetailsModel.
     *
     * The item class that is used by the DetailsGeneralModel to store and display its entries. It consists
     * of a key(label) and a value string that are both displayed in the view. Furthermore this item class
     * stores (if available) python code as a string to access it later (e.g. via a contextmenu).
     */
    class DetailsGeneralModelEntry
    {
        QString mLabel;
        QVariant mValue;
        QString mPythonGetter;
        //mSetter is the actual setter function that can be invoked in setValue
        // -> mSetter(newValue) where mSetter is for example the set_name method of a gate
        UserActionObject mObject; // must be assigned if field editable, None otherwise

    public:
        /**
         * First constructor. Primarily used to create dummy content.
         */
        DetailsGeneralModelEntry() {;}
        /**
         * Second constructor that is used for actual content of the model.
         *
         * @param label_ - The key part of the entry (usually displayed in the first column).
         * @param value_ - The value part of the entry (usually displayed in the second column).
         * @param python_ - The base python code to access the property (e.g. get_name)
         */
        DetailsGeneralModelEntry(const QString& label_,
                                 const QVariant& value_,
                                 const QString& python_ = QString())
            : mLabel(label_), mValue(value_), mPythonGetter(python_)
        {;}

        /**
         * Get the column specific data. (0 = label, 1 = value).
         *
         * @param iColumn - The column for the requested data.
         * @return The column specific data.
         */
        QVariant data(int iColumn) const;

        /**
         * Checks if a "setter method" for the entry is set so it can be modified.
         * It is displayed in the contextmenu via "Change <property>" (e.g. the name).
         *
         * @return True if entry has a setter method, false otherwise.
         */

        /**
         * Invokes the setter method for the given entry if a setter method is available.
         *
         * @param v - The new value to invoke the setter method with.
         */
        void setValue(const QString& v) const;

        /**
         * Sets the setter method for the entry (e.g. the set_name method of a module for the name entry).
         *
         * @param obj - The object value to be set.
         */
        void setObject(const UserActionObject& obj) { mObject = obj; }

        /**
         * Get the lower case version of the entry's key(label).
         *
         * @return The lower case version of the key(label).
         */
        QString lcLabel() const { return mLabel.toLower(); }

        /**
         * Get the string version of the entry's value.
         *
         * @return The string version of the entry's value.
         */
        QString textValue() const { return mValue.toString(); }

        /**
         * Returns the python getter code for the entry.
         *
         * @return The python getter code.
         */
        QString pythonGetter() const { return mPythonGetter; }

        /**
         * Checks if the text can be edited.
         *
         * @return True if it can be edited, False otherwise.
         */
        bool canEditText() const;
    };

    /**
     * @ingroup utility_widgets-selection_details
     * @brief Helper wrapper class.
     *
     * A convenience class used to gather and extract the commonly shared information
     * (such as name, type, id) of a module, gate and net in a single place. It functions
     * as sort of a wrapper class.
     */
    template <typename T> class DetailsGeneralCommonInfo
    {
    private:
        QString mItemName;
        QString mTypeName;
        u32 mId;
        Grouping* mGrouping;

        /**
         * Sets the typename that is extracted from the given module.
         * One of the overloaded functions since the class is templated.
         *
         * @param m - The module from which to extract the type.
         */
        void setTypeName(Module* m)
        {
            mTypeName = QString::fromStdString(m->get_type());
        }

        /**
         * Sets the typename that is extracted from the given gate.
         * One of the overloaded functions since the is templated.
         *
         * @param g - The gate from which to extract the type.
         */
        void setTypeName(Gate *g)
        {
            GateType* gt = g->get_type();
            if (gt) mTypeName = QString::fromStdString(gt->get_name());
        }

        /**
         * Determines the type of the given net and sets it as a variable.
         *
         * @param n - The net from which to extract the type.
         */
        void setTypeName(Net *n)
        {
            mTypeName = "Internal";
            if (gNetlist->is_global_input_net(n))
                mTypeName = "Input";
            if (gNetlist->is_global_output_net(n))
                mTypeName = "Output";
        }

    public:

        /**
         * Generates the python code that returns the given module.
         *
         * @param m - The module for which to generate the python code.
         * @return The constructed python code that returns the module.
         */
        QString getPythonBase(Module* m)
        {
            return QString("netlist.get_module_by_id(%1).").arg(m->get_id());
        }

        /**
         * Generates the python code that returns the given gate.
         *
         * @param g - The gate for which to generate the python code.
         * @return The constructed python code.
         */
        QString getPythonBase(Gate* g)
        {
            return QString("netlist.get_gate_by_id(%1).").arg(g->get_id());
        }

        /**
         * Generates the python code that returns the given net.
         *
         * @param n - The net for which to generate the python code.
         * @return - The constructed python code.
         */
        QString getPythonBase(Net* n)
        {
            return QString("netlist.get_net_by_id(%1).").arg(n->get_id());
        }

        /**
         * Get the name of the item.
         *
         * @return The name.
         */
        QString name() const
        {
            if (mItemName.isEmpty()) return "None";
            return mItemName;
        }

        /**
         * Get the grouping's name of the item.
         *
         * @return The name of the grouping.
         */
        QString grouping() const
        {
            if (!mGrouping) return "None";
            return QString::fromStdString(mGrouping->get_name());
        }

        /**
         * Get the type's name of the item.
         *
         * @return The name of the type.
         */
        QString typeName() const
        {
            if (mTypeName.isEmpty()) return "None";
            return mTypeName;
        }

        /**
         * Get the id of the item.
         *
         * @return The item's id.
         */
        u32 id() const { return mId; }

        /**
         * The constructor. The commonly shared information of all items are extracted here.
         *
         * @param item - The item to extract the information from (module, gate or net).
         */
        DetailsGeneralCommonInfo(T* item)
        {
            mItemName =  QString::fromStdString(item->get_name());
            mId       =  item->get_id();
            mGrouping =  item->get_grouping();
            setTypeName(item);
        }
    };

    /**
     * @ingroup utility_widgets-selection_details
     * @brief Model to manage the commonly shared information.
     *
     * The DetailsGeneralModel manages the general information about all 3 item types (gate, module, net)
     * so that these model can be used for each type. It behaves accordingly based on the given item type
     * in the setDummyContent (that functions as sort of a init() function that should be called after the
     * model is created) and setContent function.
     */
    class DetailsGeneralModel : public QAbstractTableModel
    {
        Q_OBJECT
        QList<DetailsGeneralModelEntry> mContent;
        int mContextIndex;
        u32 mId;
        QString mPythonBase;
        std::function<Module*(void)> mGetParentModule;

        /**
         * Appends additional rows (consisting of additional information) to the model that are unique
         * to a module-type item (such as number of gates it contains). This function is used in the
         * setContent function.
         *
         * @param m - The module from which the unique information are extracted.
         */
        void additionalInformation(Module* m);

        /**
         * Appends additional rows (consisting of additional information) to the model that are unique
         * to a gate-type item (such as the parent module). This function is used in the setContent function.
         *
         * @param g - The gate from which the unique information are extracted.
         */
        void additionalInformation(Gate* g);

        /**
         * Appends additional rows (consisting of additional information) to the model that are unique
         * to a gate-type item (there are none, it simpy assigns a setter). This function is used in
         * the setContent function.
         *
         * @param n - The net from which the unique (currently none) information are extracted.
         */
        void additionalInformation(Net* n);

        /**
         * Generates a complete python command by combining its specific python base code that is determined by
         * the item-type the model represents (module, gate, net) and an additional python sub-code given to the
         * function (e.g.: base code: netlist.get_module_by_id(id).  pyGetter: get_name();)
         *
         * @param pyGetter - The additional code that is appended to the base code.
         * @return The complete command.
         */
        QString pythonCommand(const QString& pyGetter) const;

        /**
         * Combines the name and id of a module to the form name[id] and returns the result.
         *
         * @param m - The module for which to generate the string.
         * @return The formatted string.
         */
        static QString moduleNameId(const Module* m);

    public Q_SLOTS:
        /**
         * Constructs a context menu based on the clicked item. Should be connected to a request-context-menu signal.
         *
         * @param pos - The position from where the context menu is requested.
         */
        void contextMenuRequested(const QPoint& pos);

        /**
         * Handler method for the double-click event from the view. If the clicked item was the parent-module item in
         * the gate-details-widget, the parent module is selected.
         *
         * @param inx - The modelindex that was double clicked.
         */
        void handleDoubleClick(const QModelIndex &inx);

        /**
         * Creates an input dialog in which a new value for the current context-item (that was determined by the
         * contextMenuRequesed function) can be set. When accepting the dialog, the requireUpdate signal is emitted.
         */
        void editValueTriggered();

        /**
         * Copies the raw value of the current context-item (that was determined bythe contextMenuRequested function)
         * to the clipboard.
         */
        void extractRawTriggered() const;

        /**
         * Copies the python code that was selected in the context menu created by contextMenuRequested to the clipboard.
         */
        void extractPythonTriggered() const;

    Q_SIGNALS:
        /**
         * Q_SIGNAL that is emitted when a new value in the editValueTriggered method is set.
         *
         * @param id - The id of the item that was changed.
         */
        void requireUpdate(u32 id);

    public:
        /**
         * Sets the content (fills the rows) of the model according to the type of the given item.
         *
         * @param item - The item from which to extract and set the information (valid types: module, gate, net).
         */
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

        /**
         * Appends a certain number of empty rows to the model based on the specified type.
         * Functions as a kind of init function.
         */
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

        /**
         * The constructor.
         *
         * @param parent - The model's parent.
         */
        DetailsGeneralModel(QObject* parent = nullptr);

        /**
         * This function must be overwritten so that the model functions correctly. For further information pleaser
         * refer to Qt's model-view documentation.
         */
        QVariant data(const QModelIndex &index, int role) const override;

        /**
         * This function must be overwritten so that the model functions correctly. For further information pleaser
         * refer to Qt's model-view documentation.
         */
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        /**
         * This function must be overwritten so that the model functions correctly. For further information pleaser
         * refer to Qt's model-view documentation.
         */
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    };
}
