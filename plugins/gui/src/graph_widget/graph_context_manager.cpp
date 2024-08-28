#include "gui/graph_widget/graph_context_manager.h"

#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/context_manager_widget/models/context_tree_model.h"
#include "gui/file_manager/file_manager.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/layouters/physical_graph_layouter.h"
#include "gui/graph_widget/layouters/standard_graph_layouter.h"
#include "gui/graph_widget/shaders/module_shader.h"
#include "gui/gui_globals.h"
#include "gui/settings/settings_items/settings_item_checkbox.h"
#include "gui/user_action/action_add_items_to_object.h"
#include "gui/user_action/action_create_object.h"
#include "gui/user_action/action_unfold_module.h"
#include "gui/user_action/user_action_compound.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QDir>

namespace hal
{
    SettingsItemCheckbox* GraphContextManager::sSettingNetGroupingToPins = new SettingsItemCheckbox("Net Grouping Color to Gate Pins",
                                                                                                    "graph_view/net_grp_pin",
                                                                                                    true,
                                                                                                    "Appearance:Graph View",
                                                                                                    "If set net grouping colors are also applied to input and output pins of gates");

    SettingsItemCheckbox* GraphContextManager::sSettingPanOnMiddleButton = new SettingsItemCheckbox("Pan with Middle Mouse Button",
                                                                                                    "graph_view/pan_middle_button",
                                                                                                    false,
                                                                                                    "Graph View",
                                                                                                    "If enabled middle mouse button will pan the graphics.\n"
                                                                                                    "If disabled middle mouse button can be used for rubber band selection.");

    GraphContextManager::GraphContextManager()
        : mMaxContextId(0)
    {
        mContextTreeModel = new ContextTreeModel(this);
        mSettingDebugGrid = new SettingsItemCheckbox("GUI Debug Grid",
                                                     "debug/grid",
                                                     false,
                                                     "eXpert Settings:Debug",
                                                     "Specifies whether the debug grid is displayed in the Graph View. The gird represents the scene as the layouter interprets it.");

        mSettingDumpJunction = new SettingsItemCheckbox("Dump Junction Data",
                                                     "debug/junction",
                                                     false,
                                                     "eXpert Settings:Debug",
                                                     "Dump input data from junction router to file 'junction_data.txt' for external debugging.");

        mSettingParseLayout = new SettingsItemCheckbox("Apply Parsed Position", "graph_view/layout_parse", true, "Graph View", "Use parsed verilog coordinates if available.");

        mSettingLayoutBoxes = new SettingsItemCheckbox("Optimize Box Layout", "graph_view/layout_boxes", true, "Graph View", "If disabled faster randomized placement.");
    }

    ContextDirectory* GraphContextManager::createNewDirectory(const QString& name, u32 parentId)
    {
        ContextDirectory* contextDir = mContextTreeModel->addDirectory(name, mContextTreeModel->getDirectory(parentId), ++mMaxContextId);
        return contextDir;
    }


    bool GraphContextManager::moveItem(u32 itemId, bool isDirectory, u32 parentId, int row)
    {
        ContextTreeItem* itemToMove = nullptr;
        if (isDirectory)
            itemToMove = dynamic_cast<ContextTreeItem*>(mContextTreeModel->getDirectory(itemId));
        else
            itemToMove = dynamic_cast<ContextTreeItem*>(mContextTreeModel->getContext(itemId));

        BaseTreeItem* parentItem = parentId
                ? mContextTreeModel->getDirectory(parentId)
                : mContextTreeModel->getRootItem();
        if (!parentItem || !itemToMove) return false;

        return mContextTreeModel->moveItem(itemToMove, parentItem, row);
    }



    u32 GraphContextManager::getParentId(u32 childId, bool isDirectory) const
    {
        if (isDirectory)
        {
            BaseTreeItem* bti = mContextTreeModel->getDirectory(childId);
            ContextTreeItem* parentItem = dynamic_cast<ContextTreeItem*>(bti->getParent());
            if (parentItem) return parentItem->getId();
            return 0;
        }
        GraphContext* context = getContextById(childId);
        if (!context) return 0;
        QModelIndex inx = mContextTreeModel->getIndexFromContext(context);
        if (!inx.isValid()) return 0;
        BaseTreeItem* bti =mContextTreeModel->getItemFromIndex(inx);
        if (!bti) return 0;
        ContextTreeItem* parentItem = dynamic_cast<ContextTreeItem*>(bti->getParent());
        if (parentItem) return parentItem->getId();
        return 0;
    }

    GraphContext* GraphContextManager::createNewContext(const QString& name, u32 parentId)
    {
        GraphContext* context = new GraphContext(++mMaxContextId, name);
        context->setLayouter(getDefaultLayouter(context));
        context->setShader(getDefaultShader(context));

        context->scene()->setDebugGridEnabled(mSettingDebugGrid->value().toBool());
        connect(mSettingDebugGrid, &SettingsItemCheckbox::boolChanged, context->scene(), &GraphicsScene::setDebugGridEnabled);

        mContextTreeModel->addContext(context, mContextTreeModel->getDirectory(parentId));

        Q_EMIT contextCreated(context);

        return context;
    }

    void GraphContextManager::setContextId(GraphContext* ctx, u32 ctxId)
    {
        if (!ctx || ctx->id() == ctxId)
            return;    // nothing to do
        if (getContextById(ctxId))
            return;    // id is in use
        ctx->mId = ctxId;
        if (ctxId > mMaxContextId)
            mMaxContextId = ctxId;
    }

    void GraphContextManager::openModuleInView(u32 moduleId, bool unfold)
    {
        const Module* module = gNetlist->get_module_by_id(moduleId);

        if (!module)
            return;

        GraphContext* moduleContext =
                gGraphContextManager->getContextByExclusiveModuleId(moduleId);
        if (moduleContext)
        {
            gContentManager->getContextManagerWidget()->selectViewContext(moduleContext);
            gContentManager->getContextManagerWidget()->handleOpenContextClicked();
        }
        else
        {
            UserActionCompound* act = new UserActionCompound;
            act->setUseCreatedObject();
            QString name = QString::fromStdString(module->get_name()) + " (ID: " + QString::number(moduleId) + ")";
            act->addAction(new ActionCreateObject(UserActionObjectType::ContextView, name));
            act->addAction(new ActionAddItemsToObject({module->get_id()}, {}));
            if (unfold) act->addAction(new ActionUnfoldModule(module->get_id()));
            act->exec();
            moduleContext = gGraphContextManager->getContextById(act->object().id());
            moduleContext->setDirty(false);
            moduleContext->setExclusiveModuleId(module->get_id());
        }
    }

    void GraphContextManager::openGateInView(u32 gateId)
    {
        QString name = gGraphContextManager->nextViewName("Isolated View");

        UserActionCompound* act = new UserActionCompound;
        act->setUseCreatedObject();
        act->addAction(new ActionCreateObject(UserActionObjectType::ContextView, name));
        act->addAction(new ActionAddItemsToObject({}, {gateId}));
        act->exec();
    }

    void GraphContextManager::openNetEndpointsInView(u32 netId){
        QSet<u32> allGates;

        Net* net = gNetlist->get_net_by_id(netId);

        PlacementHint plc(PlacementHint::PlacementModeType::GridPosition);
        int currentY = -(int)(net->get_num_of_sources()/2);
        for(auto endpoint : net->get_sources()) {
            u32 id = endpoint->get_gate()->get_id();
            allGates.insert(id);
            plc.addGridPosition(Node(id, Node::NodeType::Gate), {0, currentY++});
        }
        currentY = -(int)(net->get_num_of_destinations()/2);
        for(auto endpoint : net->get_destinations()) {
            u32 id = endpoint->get_gate()->get_id();
            allGates.insert(id);
            plc.addGridPosition(Node(id, Node::NodeType::Gate), {1, currentY++});
        }

        QString name = gGraphContextManager->nextViewName("Isolated View");

        UserActionCompound* act = new UserActionCompound;
        act->setUseCreatedObject();
        act->addAction(new ActionCreateObject(UserActionObjectType::ContextView, name));
        auto actionAITO = new ActionAddItemsToObject({}, allGates);
        actionAITO->setPlacementHint(plc);
        act->addAction(actionAITO);
        act->exec();
    }

    void GraphContextManager::renameGraphContextAction(GraphContext* ctx, const QString& newName)
    {
        ctx->mName = newName;

        Q_EMIT contextRenamed(ctx);
    }

    void GraphContextManager::renameContextDirectoryAction(ContextDirectory *ctxDir, const QString &newName)
    {
        ctxDir->setName(newName);

        Q_EMIT directoryRenamed(ctxDir);
    }

    void GraphContextManager::deleteGraphContext(GraphContext* ctx)
    {
        Q_EMIT deletingContext(ctx);

        mContextTreeModel->removeContext(ctx);

        delete ctx;
    }

    void GraphContextManager::deleteContextDirectory(ContextDirectory *ctxDir)
    {
        Q_EMIT deletingDirectory(ctxDir);

        mContextTreeModel->removeDirectory(ctxDir);

        delete ctxDir;
    }

    QVector<GraphContext*> GraphContextManager::getContexts() const
    {
        return mContextTreeModel->list();
    }

    GraphContext* GraphContextManager::getContextById(u32 id) const
    {
        for (GraphContext* ctx : mContextTreeModel->list())
        {
            if (ctx->id() == id)
                return ctx;
        }
        return nullptr;
    }

    ContextDirectory *GraphContextManager::getDirectoryById(u32 id) const
    {
        BaseTreeItem* bti = mContextTreeModel->getDirectory(id);
        if (bti)
        {
            ContextDirectory* directory = static_cast<ContextTreeItem*>(bti)->directory();
            if (directory) return directory;
        }

        return nullptr;
    }

    GraphContext* GraphContextManager::getCleanContext(const QString& name) const
    {
        for (GraphContext* ctx : mContextTreeModel->list())
        {
            if (ctx->name() == name && !ctx->isDirty())
                return ctx;
        }
        return nullptr;
    }

    GraphContext* GraphContextManager::getContextByExclusiveModuleId(u32 module_id) const
    {
        for (GraphContext* ctx : mContextTreeModel->list())
        {
            if (ctx->getExclusiveModuleId() == module_id)
                return ctx;
        }
        return nullptr;
    }

    QString GraphContextManager::nextViewName(const QString& prefix) const
    {
        int cnt = 0;

        for (;;)
        {
            QString name = QString("%1 %2").arg(prefix).arg(++cnt);
            if (!contextWithNameExists(name)) return name;
        }
    }

    bool GraphContextManager::contextWithNameExists(const QString& name) const
    {
        for (GraphContext* ctx : mContextTreeModel->list())
        {
            if (ctx->name() == name)
            {
                return true;
            }
        }
        return false;
    }

    void GraphContextManager::handleModuleCreated(Module* m) const
    {
        for (GraphContext* context : mContextTreeModel->list())
            if (context->getSpecialUpdate())
            {
                context->add({m->get_id()}, {});
                context->setSpecialUpdate(false);
            }
    }

    void GraphContextManager::handleModuleRemoved(Module* m)
    {
        for (GraphContext* context : mContextTreeModel->list())
        {
            if (context->getExclusiveModuleId() == m->get_id())
            {
                context->setExclusiveModuleId(0, false);
                deleteGraphContext(context);
            }
            else if (context->modules().contains(m->get_id()))
            {
                context->remove({m->get_id()}, {});

                if (context->empty() || context->willBeEmptied())
                    deleteGraphContext(context);
            }
        }
    }

    void GraphContextManager::handleModuleNameChanged(Module* m) const
    {
        for (GraphContext* context : mContextTreeModel->list())
            if (context->modules().contains(m->get_id()))
                context->scheduleSceneUpdate();
    }

    void GraphContextManager::handleModuleTypeChanged(Module* m) const
    {
        for (GraphContext* context : mContextTreeModel->list())
            if (context->modules().contains(m->get_id()))
                context->scheduleSceneUpdate();
    }

    void GraphContextManager::handleModuleColorChanged(Module* m) const
    {
        auto gates = m->get_gates();
        QSet<u32> gateIDs;
        for (auto g : gates)
            gateIDs.insert(g->get_id());
        for (GraphContext* context : mContextTreeModel->list())
            if (context->modules().contains(m->get_id())    // contains module
                || context->gates().intersects(gateIDs))    // contains gate from module
                context->scheduleSceneUpdate();
        // a context can contain a gate from a module if it is showing the module
        // or if it's showing a parent and the module is unfolded
    }

    void GraphContextManager::handleModuleSubmoduleAdded(Module* m, const u32 added_module) const
    {
        //        dump("ModuleSubmoduleAdded", m->get_id(), added_module);

        QList<u32> module_ids  = {};
        Module* current_module = m;

        do
        {
            module_ids.append(current_module->get_id());
            current_module = current_module->get_parent_module();
        } while (current_module);

        for (GraphContext* context : mContextTreeModel->list())
        {
            if (context->isShowingFoldedTopModule()) continue;
            if (context->isShowingModule(m->get_id(), {added_module}, {}, {}, {}) && !context->isShowingModule(added_module, {}, {}, {}, {}))
                context->add({added_module}, {});
            else
                context->testIfAffected(m->get_id(), &added_module, nullptr);

            // When the module is unfolded and was moved to another folded module visible in view,
            // remove all gates and submodules of added_module from view
            if (context->isShowingModule(added_module, {}, {}, {}, {}))
            {
                QSet<u32> modules = context->modules();
                modules.remove(added_module);

                // modules    : all modules visible in graph context view except recently added
                // module_ids : all anchestors of recently added module
                // algorithm  : remove 'recently added' module from view if it was moved to
                //              (child or grandchild of) another module visible in view
                for (u32 id : module_ids)
                    if (modules.contains(id))
                    {
                        context->removeModuleContents(added_module);
                        return;
                    }
            }
        }
    }

    void GraphContextManager::handleModuleSubmoduleRemoved(Module* m, const u32 removed_module)
    {
        // FIXME this also triggers on module deletion (not only moving)
        // and collides with handleModuleRemoved
        //        dump("ModuleSubmoduleRemoved", m->get_id(), removed_module);

        for (GraphContext* context : mContextTreeModel->list())
        {
            if (context->isShowingFoldedTopModule()) continue;
            if (context->isScheduledRemove(Node(removed_module,Node::Module)) ||
                    context->isShowingModule(m->get_id(), {}, {}, {removed_module}, {}))
                context->remove({removed_module}, {});
            else
                context->testIfAffected(m->get_id(), &removed_module, nullptr);

            if (context->empty() || context->willBeEmptied())
                deleteGraphContext(context);
        }
    }

    void GraphContextManager::handleModuleGateAssigned(Module* m, const u32 inserted_gate) const
    {
        //        dump("ModuleGateAssigned", m->get_id(), inserted_gate);

        QList<u32> module_ids  = {};
        Module* current_module = m;

        do
        {
            module_ids.append(current_module->get_id());
            current_module = current_module->get_parent_module();
        } while (current_module);

        for (GraphContext* context : mContextTreeModel->list())
        {
            if (context->isShowingFoldedTopModule()) continue;
            if (context->isShowingModule(m->get_id(), {}, {inserted_gate}, {}, {}))
                context->add({}, {inserted_gate});
            else
                context->testIfAffected(m->get_id(), nullptr, &inserted_gate);

            if (context->gates().contains(inserted_gate))
            {
                QSet<u32> modules = context->modules();

                for (u32 id : module_ids)
                    if (modules.contains(id))
                    {
                        context->remove({}, {inserted_gate});
                        break;
                    }
            }
        }
    }

    void GraphContextManager::handleModuleGateRemoved(Module* m, const u32 removed_gate)
    {
        //        dump("ModuleGateRemoved", m->get_id(), removed_gate);
        for (GraphContext* context : mContextTreeModel->list())
        {
            if (context->isShowingFoldedTopModule()) continue;
            if (context->isScheduledRemove(Node(removed_gate,Node::Gate)) ||
                    context->isShowingModule(m->get_id(), {}, {}, {}, {removed_gate}))
            {
                context->remove({}, {removed_gate});
                if (context->empty() || context->willBeEmptied())
                {
                    deleteGraphContext(context);
                }
                // when the module is empty, add the empty folded module to the view
                else if (m->get_gates().empty() && m->get_submodules().empty())
                {
                    context->add({m->get_id()}, {});
                }
            }
            // if a module is unfolded, then the gate is not deleted from the view
            // but the color of the gate changes to its new parent's color

            else
                context->testIfAffected(m->get_id(), nullptr, &removed_gate);

            /// new code line above should cover commented lines below
            /// else if (context->gates().contains(removed_gate))
            ///    context->scheduleSceneUpdate();
        }
    }

    void GraphContextManager::dump(const QString& title, u32 mid, u32 xid) const
    {
        QTextStream xout(stdout, QIODevice::WriteOnly);
        xout << "===" << title << "===" << mid << "===" << xid << "===\n";
        for (Module* m : gNetlist->get_modules())
        {
            xout << "M" << m->get_id() << "   SM:";
            for (Module* sm : m->get_submodules())
                xout << " " << sm->get_id();
            xout << "   G:";
            for (Gate* g : m->get_gates())
                xout << " " << g->get_id();
            xout << "\n";
        }
        xout << "gats:";
        for (Gate* g : gNetlist->get_gates())
            xout << " " << g->get_id();
        xout << "\n";

        for (GraphContext* ctx : mContextTreeModel->list())
        {
            xout << "ctx " << ctx->id() << ":";
            for (hal::Node nd : ctx->getLayouter()->positionToNodeMap().values())
            {
                xout << " " << (nd.type() == Node::Module ? 'm' : 'g') << nd.id();
            }
            xout << "\n";
        }
        xout << "-------\n";
    }

    void GraphContextManager::handleModulePortsChanged(Module* m, PinEvent pev, u32 pgid)
    {
        Q_UNUSED(pev);
        Q_UNUSED(pgid);
        for (GraphContext* context : mContextTreeModel->list())
            if (context->modules().contains(m->get_id()))
            {
                context->updateNets();
                context->scheduleSceneUpdate();
            }
    }

    void GraphContextManager::handleGateRemoved(Gate* g) const
    {
        for (GraphContext* context : mContextTreeModel->list())
            if (context->gates().contains(g->get_id()))
                context->remove({}, {g->get_id()});
    }

    void GraphContextManager::handleGateNameChanged(Gate* g) const
    {
        for (GraphContext* context : mContextTreeModel->list())
            if (context->gates().contains(g->get_id()))
                context->scheduleSceneUpdate();
    }

    void GraphContextManager::handleNetCreated(Net* n) const
    {
        Q_UNUSED(n)

        // CAN NETS BE CREATED WITH SRC AND DST INFORMATION ?
        // IF NOT THIS EVENT DOESNT NEED TO BE HANDLED
    }

    void GraphContextManager::handleNetRemoved(Net* n) const
    {
        for (GraphContext* context : mContextTreeModel->list())
            if (context->nets().contains(n->get_id()))
                context->scheduleSceneUpdate();
    }

    void GraphContextManager::handleNetNameChanged(Net* n) const
    {
        for (GraphContext* context : mContextTreeModel->list())
            if (context->nets().contains(n->get_id()))
                context->scheduleSceneUpdate();
    }

    void GraphContextManager::handleNetSourceAdded(Net* n, const u32 src_gate_id) const
    {
        for (GraphContext* context : mContextTreeModel->list())
        {
            if (context->nets().contains(n->get_id()) || context->gates().contains(src_gate_id))
            {
                // forcibly apply changes since nets need to be recalculated
                context->applyChanges();
                context->scheduleSceneUpdate();
            }
        }
    }

    void GraphContextManager::handleNetSourceRemoved(Net* n, const u32 src_gate_id) const
    {
        UNUSED(src_gate_id);

        for (GraphContext* context : mContextTreeModel->list())
        {
            if (context->nets().contains(n->get_id()))
            {
                // forcibly apply changes since nets need to be recalculated
                context->applyChanges();
                context->scheduleSceneUpdate();
            }
        }
    }

    void GraphContextManager::handleNetDestinationAdded(Net* n, const u32 dst_gate_id) const
    {
        for (GraphContext* context : mContextTreeModel->list())
        {
            if (context->nets().contains(n->get_id()) || context->gates().contains(dst_gate_id))
            {
                // forcibly apply changes since nets need to be recalculated
                context->applyChanges();
                context->scheduleSceneUpdate();
            }
        }
    }

    void GraphContextManager::handleNetDestinationRemoved(Net* n, const u32 dst_gate_id) const
    {
        UNUSED(dst_gate_id);

        for (GraphContext* context : mContextTreeModel->list())
        {
            if (context->nets().contains(n->get_id()))
            {
                // forcibly apply changes since nets need to be recalculated
                context->applyChanges();
                context->scheduleSceneUpdate();
            }
        }
    }

    void GraphContextManager::handleMarkedGlobalInput(u32 mNetId)
    {
        for (GraphContext* context : mContextTreeModel->list())
        {
            if (context->nets().contains(mNetId) || context->isShowingNetDestination(mNetId))
            {
                context->applyChanges();
                context->scheduleSceneUpdate();
            }
        }
    }

    void GraphContextManager::handleMarkedGlobalOutput(u32 mNetId)
    {
        for (GraphContext* context : mContextTreeModel->list())
        {
            if (context->nets().contains(mNetId) || context->isShowingNetSource(mNetId))
            {
                context->applyChanges();
                context->scheduleSceneUpdate();
            }
        }
    }

    void GraphContextManager::handleUnmarkedGlobalInput(u32 mNetId)
    {
        for (GraphContext* context : mContextTreeModel->list())
            if (context->nets().contains(mNetId))
            {
                context->applyChanges();
                context->scheduleSceneUpdate();
            }
    }

    void GraphContextManager::handleUnmarkedGlobalOutput(u32 mNetId)
    {
        for (GraphContext* context : mContextTreeModel->list())
            if (context->nets().contains(mNetId))
            {
                context->applyChanges();
                context->scheduleSceneUpdate();
            }
    }

    GraphLayouter* GraphContextManager::getDefaultLayouter(GraphContext* const context) const
    {
        StandardGraphLayouter* layouter = new StandardGraphLayouter(context);
        layouter->setDumpJunctionEnabled(mSettingDumpJunction->value().toBool());
        layouter->setParseLayoutEnabled(mSettingParseLayout->value().toBool());
        layouter->setLayoutBoxesEnabled(mSettingLayoutBoxes->value().toBool());

        connect(mSettingDumpJunction, &SettingsItemCheckbox::boolChanged, layouter, &GraphLayouter::setDumpJunctionEnabled);
        connect(mSettingParseLayout, &SettingsItemCheckbox::boolChanged, layouter, &StandardGraphLayouter::setParseLayoutEnabled);
        connect(mSettingLayoutBoxes, &SettingsItemCheckbox::boolChanged, layouter, &StandardGraphLayouter::setLayoutBoxesEnabled);

        return layouter;
    }

    GraphShader* GraphContextManager::getDefaultShader(GraphContext* const context) const
    {
        return new ModuleShader(context);
    }

    ContextTreeModel* GraphContextManager::getContextTreeModel() const
    {
        return mContextTreeModel;
    }

    void GraphContextManager::clear()
    {
        for (GraphContext* context : mContextTreeModel->list())
            delete context;

        mContextTreeModel->clear();
    }

    GraphContext* GraphContextManager::restoreFromFile(const QString& filename)
    {
        QFile jsFile(filename);
        if (!jsFile.open(QIODevice::ReadOnly))
            return nullptr;
        QJsonDocument jsonDoc   = QJsonDocument::fromJson(jsFile.readAll());
        const QJsonObject& json = jsonDoc.object();
        GraphContext* firstContext = nullptr;
        GraphContext* selectedContext = nullptr;

        if (json.contains("directories") && json["directories"].isArray())
        {
            QJsonArray jsonDirectories = json["directories"].toArray();
            int ndir = jsonDirectories.size();
            for (int idir = 0; idir < ndir; idir++)
            {
                QJsonObject jsondir = jsonDirectories.at(idir).toObject();
                if (!jsondir.contains("id") || !jsondir["id"].isDouble())
                    continue;
                u32 dirId = jsondir["id"].toInt();
                if (!jsondir.contains("name") || !jsondir["name"].isString())
                    continue;
                QString dirName = jsondir["name"].toString();
                if (!jsondir.contains("parentId"))
                    continue;
                u32 dirParentId = jsondir["parentId"].toInt();

                BaseTreeItem* dirParent = mContextTreeModel->getRootItem();

                if (dirParentId != 0)
                   dirParent =  mContextTreeModel->getDirectory(dirParentId);

                if (dirId < 0 || dirId > 0x7FFFFFFF)
                    dirId = ++mMaxContextId;

                mContextTreeModel->addDirectory(dirName, dirParent, dirId);
            }
        }
        if (json.contains("views") && json["views"].isArray())
        {
            QJsonArray jsonViews = json["views"].toArray();
            int nviews           = jsonViews.size();
            for (int iview = 0; iview < nviews; iview++)
            {
                QJsonObject jsonView = jsonViews.at(iview).toObject();
                if (!jsonView.contains("id") || !jsonView["id"].isDouble())
                    continue;
                u32 viewId = jsonView["id"].toInt();
                if (!jsonView.contains("name") || !jsonView["name"].isString())
                    continue;
                QString viewName = jsonView["name"].toString();
                if (viewId > mMaxContextId)
                    mMaxContextId = viewId;
                int visibleFlag = 1; // default to visible before flag was invented
                if (jsonView.contains("visible"))
                    visibleFlag = jsonView["visible"].toInt();

                u32 viewParentId = 0;
                if (!jsonView.contains("parentId"))
                    viewParentId = jsonView["parentId"].toInt();

                BaseTreeItem* viewParent = mContextTreeModel->getRootItem();

                if (viewParentId != 0) {
                    viewParent = mContextTreeModel->getDirectory(viewParentId);
                }


                GraphContext* context = gGraphContextManager->getContextById(viewId);
                if (context)
                {
                    // load view in existing context
                    context->clear();
                    if (viewName != context->mName)
                        renameGraphContextAction(context,viewName);
                    if (!context->readFromFile(jsonView))
                    {
                        log_warning("gui", "failed to set up existing context ID={} from view file {}.", context->id(), filename.toStdString());
                        continue;
                    }
                }
                else
                {
                    // create new context
                    context = new GraphContext(viewId, viewName);
                    context->setLayouter(getDefaultLayouter(context));
                    context->setShader(getDefaultShader(context));
                    // context->setExclusiveModuleId(exclusiveModuleId); TODO exclusive module ID
                    context->scene()->setDebugGridEnabled(mSettingDebugGrid->value().toBool());
                    connect(mSettingDebugGrid, &SettingsItemCheckbox::boolChanged, context->scene(), &GraphicsScene::setDebugGridEnabled);

                    if (!context->readFromFile(jsonView))
                    {
                        log_warning("gui", "failed to create context context ID={} view file {}.", context->id(), filename.toStdString());
                        deleteGraphContext(context);
                        continue;
                    }

                    mContextTreeModel->addContext(context, viewParent);
                    if (visibleFlag)
                        Q_EMIT contextCreated(context);
                }

                if (jsonView.contains("exclusiveModuleId"))
                    context->setExclusiveModuleId(jsonView["exclusiveModuleId"].toInt(),false);
                if (jsonView.contains("selected"))
                    selectedContext = context;
                if (visibleFlag==2)
                    selectedContext = context;
                if (!firstContext)
                    firstContext = context;
                context->setDirty(false);
            }
        }
        if (selectedContext) return selectedContext;
        return firstContext;
    }

    bool GraphContextManager::handleSaveTriggered(const QString& filename)
    {
        QFile jsFile(filename);
        if (!jsFile.open(QIODevice::WriteOnly))
            return false;

        QJsonObject json;
        QJsonArray jsonViews;
        for (GraphContext* context : mContextTreeModel->list())
            {
                BaseTreeItem* parent = mContextTreeModel->getItemFromIndex(mContextTreeModel->getIndexFromContext(context))->getParent();
                int parentId = 0;
                if (parent != mContextTreeModel->getRootItem())
                    parentId = static_cast<ContextTreeItem*>(parent)->directory()->id();

                QJsonObject jsonView;
                context->writeToFile(jsonView, parentId);
                jsonViews.append(jsonView);
            }
        json["views"] = jsonViews;

        QJsonArray jsonDirectories;
        for (ContextDirectory* directory : mContextTreeModel->directoryList())
            {
                QJsonObject jsonDirectory;
                directory->writeToFile(jsonDirectory);
                jsonDirectories.append(jsonDirectory);
            }
        json["directories"] = jsonDirectories;
        return (jsFile.write(QJsonDocument(json).toJson(QJsonDocument::Compact)) >= 0); // neg return value indicates error
    }
}    // namespace hal
