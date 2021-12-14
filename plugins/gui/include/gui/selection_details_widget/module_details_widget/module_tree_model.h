#include "gui/basic_tree_model/base_tree_model.h"
#include "hal_core/defines.h"
#include <QIcon>
#include <QMap>

namespace hal
{
    class Module;
    class Gate;
    class Net;
    class TreeItem;

    class ModuleTreeModel : public BaseTreeModel
    {
        Q_OBJECT
    public:

        //metatype declaration at the end of file
        enum class itemType {module = 0, gate = 1};

        ModuleTreeModel(QObject* parent = nullptr);

        ~ModuleTreeModel();

        void setModule(Module* m);

        void clear() override;

        /** @name Overwritten model functions
         */
        ///@{

        /**
          * Overwritten Qt function that is necessary for the model. For further information pleaser
          * refer to the Qt documentation.
          */
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        ///@}

        /**
         * Get the type (enum) of a given item.
         *
         * @param item - The item for which the type is requested.
         * @return The item's type.
         */
        itemType getTypeOfItem(TreeItem* item) const;

        /**
         * Disconnects all events from the model. Can be called to increase performance when
         * no module is displayed.
         */
        void disconnectEvents();

        /**
         * Connects all events to the model. When setting a module, all events will be
         * automatically conneceted.
         */
        void connectEvents();

        //Column identifier
        static const int sNameColumn = 0;
        static const int sIdColumn = 1;
        static const int sTypeColumn = 2;

    Q_SIGNALS:

        /**
         * Signal that is emitted when the number of direct submodules changed
         * in the case the displayed content is displayed by using the method
         * setModule().
         *
         * @param newNumber - The new number of direct submodules.
         */
        void numberOfSubmodulesChanged(const int newNumber);


    private:
        //Note: make these somehow static (does not work with pointer...?)
        QIcon mModuleIcon;
        QIcon mGateIcon;

        QString mKeyItemType = "type";
        QString mKeyRepId = "id";
        int mThreshold = 1;
        //only for "all events", specific events that are disabled by event-guard handlers
        //musst be handled there (disconnect specific events at begin, connect them at end)
        bool mEventsConnected = false;

        int mModId;
        QMap<Module*, TreeItem*> mModuleToTreeitems;
        QMap<Gate*, TreeItem*> mGateToTreeitems;

        //necessary because setModule uses beginResetModel (should not be called by each recursive iteration)
        void moduleRecursive(Module* mod, TreeItem* modItem);

        //perhaps more performance instead of setting the whole displayed module anew
        void updateGatesOfModule(Module* mod);

        /**
         * Utility function to determine the displayed icon for a given item
         *
         * @param item - The requested item.
         * @return A module, net, or gate icon depending on the item's type.
         */
        QIcon getIconFromItem(TreeItem* item) const;

        void clearOwnStructures();

        //guards
        void handleModuleGatesAssignBegin(Module* m, u32 associated_data);
        void handleModuleGatesAssignEnd(Module* m, u32 associated_data);
        void handleModuleGatesRemoveBegin(Module* m, u32 associated_data);
        void handleModuleGatesRemoveEnd(Module* m, u32 associated_data);

        //actual functions
        void handleModuleSubmoduleAdded(Module* m, u32 added_module);
        void handleModuleSubmoduleRemoved(Module* m, u32 removed_module);
        void handleModuleGateAssigned(Module* m, u32 assigned_gate);
        void handleModuleGateRemoved(Module* m, u32 removed_gate);
        void handleModuleRemoved(Module* m);

        void handleGateNameChanged(Gate* g);
        void handleModuleNameChanged(Module* m);
        void handleModuleTypeChanged(Module* m);
    };

}

Q_DECLARE_METATYPE(hal::ModuleTreeModel::itemType)
