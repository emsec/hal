#include "gui/graph_widget/graph_context_manager.h"

#include "gui/context_manager_widget/models/context_table_model.h"
#include "gui/file_manager/file_manager.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/layouters/physical_graph_layouter.h"
#include "gui/graph_widget/layouters/standard_graph_layouter.h"
#include "gui/graph_widget/shaders/module_shader.h"
#include "gui/gui_globals.h"
#include "gui/settings/settings_items/settings_item_checkbox.h"
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

    GraphContextManager::GraphContextManager() : mContextTableModel(new ContextTableModel()), mMaxContextId(0)
    {
        mSettingDebugGrid = new SettingsItemCheckbox("GUI Debug Grid",
                                                     "debug/grid",
                                                     false,
                                                     "eXpert Settings:Debug",
                                                     "Specifies whether the debug grid is displayed in the Graph View. The gird represents the scene as the layouter interprets it.");

        mSettingNetLayout = new SettingsItemCheckbox("Optimize Net Layout", "graph_view/layout_nets", true, "Graph View", "Net optimization - not fully tested yet.");

        mSettingParseLayout = new SettingsItemCheckbox("Apply Parsed Position", "graph_view/layout_parse", true, "Graph View", "Use parsed verilog coordinates if available.");

        mSettingLayoutBoxes = new SettingsItemCheckbox("Optimize Box Layout", "graph_view/layout_boxes", true, "Graph View", "If disabled faster randomized placement.");
    }

    GraphContext* GraphContextManager::createNewContext(const QString& name)
    {
        GraphContext* context = new GraphContext(++mMaxContextId, name);
        context->setLayouter(getDefaultLayouter(context));
        context->setShader(getDefaultShader(context));

        context->scene()->setDebugGridEnabled(mSettingDebugGrid->value().toBool());
        connect(mSettingDebugGrid, &SettingsItemCheckbox::boolChanged, context->scene(), &GraphicsScene::setDebugGridEnabled);

        mContextTableModel->beginInsertContext(context);
        mContextTableModel->addContext(context);
        mContextTableModel->endInsertContext();

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

    void GraphContextManager::renameGraphContextAction(GraphContext* ctx, const QString& newName)
    {
        ctx->mName = newName;

        Q_EMIT contextRenamed(ctx);
    }

    void GraphContextManager::deleteGraphContext(GraphContext* ctx)
    {
        Q_EMIT deletingContext(ctx);

        mContextTableModel->beginRemoveContext(ctx);
        mContextTableModel->removeContext(ctx);
        mContextTableModel->endRemoveContext();

        delete ctx;
    }

    QVector<GraphContext*> GraphContextManager::getContexts() const
    {
        return mContextTableModel->list();
    }

    GraphContext* GraphContextManager::getContextById(u32 id) const
    {
        for (GraphContext* ctx : mContextTableModel->list())
        {
            if (ctx->id() == id)
                return ctx;
        }
        return nullptr;
    }

    GraphContext* GraphContextManager::getCleanContext(const QString& name) const
    {
        for (GraphContext* ctx : mContextTableModel->list())
        {
            if (ctx->name() == name && !ctx->isDirty())
                return ctx;
        }
        return nullptr;
    }

    GraphContext* GraphContextManager::getContextByExclusiveModuleId(u32 module_id) const
    {
        for (GraphContext* ctx : mContextTableModel->list())
        {
            if (ctx->getExclusiveModuleId() == module_id)
                return ctx;
        }
        return nullptr;
    }

    bool GraphContextManager::contextWithNameExists(const QString& name) const
    {
        for (GraphContext* ctx : mContextTableModel->list())
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
        for (GraphContext* context : mContextTableModel->list())
            if (context->getSpecialUpdate())
            {
                context->add({m->get_id()}, {});
                context->setSpecialUpdate(false);
            }
    }

    void GraphContextManager::handleModuleRemoved(Module* m)
    {
        for (GraphContext* context : mContextTableModel->list())
            if (context->modules().contains(m->get_id()))
            {
                if (context->getExclusiveModuleId() == m->get_id())
                    context->setExclusiveModuleId(0, false);

                context->remove({m->get_id()}, {});

                if (context->empty() || context->willBeEmptied())
                    deleteGraphContext(context);
            }
    }

    void GraphContextManager::handleModuleNameChanged(Module* m) const
    {
        for (GraphContext* context : mContextTableModel->list())
            if (context->modules().contains(m->get_id()))
                context->scheduleSceneUpdate();
    }

    void GraphContextManager::handleModuleTypeChanged(Module* m) const
    {
        for (GraphContext* context : mContextTableModel->list())
            if (context->modules().contains(m->get_id()))
                context->scheduleSceneUpdate();
    }

    void GraphContextManager::handleModuleColorChanged(Module* m) const
    {
        auto gates = m->get_gates();
        QSet<u32> gateIDs;
        for (auto g : gates)
            gateIDs.insert(g->get_id());
        for (GraphContext* context : mContextTableModel->list())
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

        for (GraphContext* context : mContextTableModel->list())
        {
            if (context->isShowingModule(m->get_id(), {added_module}, {}, {}, {}, false) && !context->isShowingModule(added_module, {}, {}, {}, {}, false))
                context->add({added_module}, {});
            else
                context->testIfAffected(m->get_id(), &added_module, nullptr);

            // When the module is unfolded and was moved to another folded module visible in view,
            // remove all gates and submodules of added_module from view
            if (context->isShowingModule(added_module, {}, {}, {}, {}, false))
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

        for (GraphContext* context : mContextTableModel->list())
        {
            if (context->isShowingModule(m->get_id(), {}, {}, {removed_module}, {}, false))
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

        for (GraphContext* context : mContextTableModel->list())
        {
            if (context->isShowingModule(m->get_id(), {}, {inserted_gate}, {}, {}, false))
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
        for (GraphContext* context : mContextTableModel->list())
        {
            if (context->isShowingModule(m->get_id(), {}, {}, {}, {removed_gate}, false))
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

        for (GraphContext* ctx : mContextTableModel->list())
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

    void GraphContextManager::handleModulePortsChanged(Module* m)
    {
        for (GraphContext* context : mContextTableModel->list())
            if (context->modules().contains(m->get_id()))
            {
                context->updateNets();
                context->scheduleSceneUpdate();
            }
    }

    void GraphContextManager::handleGateRemoved(Gate* g) const
    {
        for (GraphContext* context : mContextTableModel->list())
            if (context->gates().contains(g->get_id()))
                context->remove({}, {g->get_id()});
    }

    void GraphContextManager::handleGateNameChanged(Gate* g) const
    {
        for (GraphContext* context : mContextTableModel->list())
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
        for (GraphContext* context : mContextTableModel->list())
            if (context->nets().contains(n->get_id()))
                context->scheduleSceneUpdate();
    }

    void GraphContextManager::handleNetNameChanged(Net* n) const
    {
        for (GraphContext* context : mContextTableModel->list())
            if (context->nets().contains(n->get_id()))
                context->scheduleSceneUpdate();
    }

    void GraphContextManager::handleNetSourceAdded(Net* n, const u32 src_gate_id) const
    {
        for (GraphContext* context : mContextTableModel->list())
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

        for (GraphContext* context : mContextTableModel->list())
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
        for (GraphContext* context : mContextTableModel->list())
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

        for (GraphContext* context : mContextTableModel->list())
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
        for (GraphContext* context : mContextTableModel->list())
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
        for (GraphContext* context : mContextTableModel->list())
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
        for (GraphContext* context : mContextTableModel->list())
            if (context->nets().contains(mNetId))
            {
                context->applyChanges();
                context->scheduleSceneUpdate();
            }
    }

    void GraphContextManager::handleUnmarkedGlobalOutput(u32 mNetId)
    {
        for (GraphContext* context : mContextTableModel->list())
            if (context->nets().contains(mNetId))
            {
                context->applyChanges();
                context->scheduleSceneUpdate();
            }
    }

    GraphLayouter* GraphContextManager::getDefaultLayouter(GraphContext* const context) const
    {
        StandardGraphLayouter* layouter = new StandardGraphLayouter(context);
        layouter->setOptimizeNetLayoutEnabled(mSettingNetLayout->value().toBool());
        layouter->setParseLayoutEnabled(mSettingParseLayout->value().toBool());
        layouter->setLayoutBoxesEnabled(mSettingLayoutBoxes->value().toBool());

        connect(mSettingNetLayout, &SettingsItemCheckbox::boolChanged, layouter, &GraphLayouter::setOptimizeNetLayoutEnabled);
        connect(mSettingParseLayout, &SettingsItemCheckbox::boolChanged, layouter, &StandardGraphLayouter::setParseLayoutEnabled);
        connect(mSettingLayoutBoxes, &SettingsItemCheckbox::boolChanged, layouter, &StandardGraphLayouter::setLayoutBoxesEnabled);

        return layouter;
    }

    GraphShader* GraphContextManager::getDefaultShader(GraphContext* const context) const
    {
        return new ModuleShader(context);
    }

    ContextTableModel* GraphContextManager::getContextTableModel() const
    {
        return mContextTableModel;
    }

    void GraphContextManager::clear()
    {
        for (GraphContext* context : mContextTableModel->list())
            delete context;

        mContextTableModel->clear();
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

                    mContextTableModel->beginInsertContext(context);
                    mContextTableModel->addContext(context);
                    mContextTableModel->endInsertContext();
                    Q_EMIT contextCreated(context);
                }

                if (jsonView.contains("exclusiveModuleId"))
                    context->setExclusiveModuleId(jsonView["exclusiveModuleId"].toInt(),false);
                if (jsonView.contains("selected"))
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
        for (GraphContext* context : mContextTableModel->list())
            {
                QJsonObject jsonView;
                context->writeToFile(jsonView);
                jsonViews.append(jsonView);
            }
        json["views"] = jsonViews;
        return (jsFile.write(QJsonDocument(json).toJson(QJsonDocument::Compact)) >= 0); // neg return value indicates error
    }
}    // namespace hal
