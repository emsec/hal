#include "gui/graph_widget/graph_context_manager.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/layouters/physical_graph_layouter.h"
#include "gui/graph_widget/layouters/standard_graph_layouter.h"
#include "gui/graph_widget/shaders/module_shader.h"
#include "gui/context_manager_widget/models/context_table_model.h"
#include "gui/file_manager/file_manager.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace hal
{
    GraphContextManager::GraphContextManager()
        : mContextTableModel(new ContextTableModel()), mMaxContextId(0)
    {;}

    GraphContext* GraphContextManager::createNewContext(const QString& name)
    {
        GraphContext* context = new GraphContext(++mMaxContextId, name);
        context->setLayouter(getDefaultLayouter(context));
        context->setShader(getDefaultShader(context));

        mContextTableModel->beginInsertContext(context);
        mContextTableModel->addContext(context);
        mContextTableModel->endInsertContext();

        Q_EMIT contextCreated(context);

        return context;
    }

    void GraphContextManager::renameGraphContext(GraphContext* ctx, const QString& new_name)
    {
        ctx->mName = new_name;

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

    void GraphContextManager::handleModuleRemoved(Module* m)
    {
        for (GraphContext* context : mContextTableModel->list())
            if (context->modules().contains(m->get_id()))
                context->remove({m->get_id()}, {});
    }

    void GraphContextManager::handleModuleNameChanged(Module* m) const
    {
        for (GraphContext* context : mContextTableModel->list())
            if (context->modules().contains(m->get_id()))
                context->scheduleSceneUpdate();
    }

    void GraphContextManager::handleModuleTypeChanged(Module *m) const
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
        for (GraphContext* context : mContextTableModel->list())
            if (context->isShowingModule(m->get_id(), {added_module}, {}, {}, {}))
                context->add({added_module}, {});
            else context->testIfAffected(m->get_id(), &added_module, nullptr);
    }

    void GraphContextManager::handleModuleSubmoduleRemoved(Module* m, const u32 removed_module)
    {
        // FIXME this also triggers on module deletion (not only moving)
        // and collides with handleModuleRemoved
        for (GraphContext* context : mContextTableModel->list())
            if (context->isShowingModule(m->get_id(), {}, {}, {removed_module}, {}))
            {
                context->remove({removed_module}, {});
                if (context->empty())
                {
                    deleteGraphContext(context);
                }
            }
            else context->testIfAffected(m->get_id(), &removed_module, nullptr);
    }

    void GraphContextManager::handleModuleGateAssigned(Module* m, const u32 inserted_gate) const
    {
        for (GraphContext* context : mContextTableModel->list())
            if (context->isShowingModule(m->get_id(), {}, {inserted_gate}, {}, {}))
                context->add({}, {inserted_gate});
            else context->testIfAffected(m->get_id(), nullptr, &inserted_gate);
    }

    void GraphContextManager::handleModuleGateRemoved(Module* m, const u32 removed_gate)
    {
        for (GraphContext* context : mContextTableModel->list())
        {
            if (context->isShowingModule(m->get_id(), {}, {}, {}, {removed_gate}))
            {
                context->remove({}, {removed_gate});
                if (context->empty())
                {
                    deleteGraphContext(context);
                }
            }
            // if a module is unfolded, then the gate is not deleted from the view
            // but the color of the gate changes to its new parent's color


            else context->testIfAffected(m->get_id(), nullptr, &removed_gate);

            /// new code line above should cover commented lines below
            /// else if (context->gates().contains(removed_gate))
            ///    context->scheduleSceneUpdate();
        }
    }

    void GraphContextManager::handleModuleInputPortNameChanged(Module* m, const u32 net)
    {
        Q_UNUSED(net);
        for (GraphContext* context : mContextTableModel->list())
            if (context->modules().contains(m->get_id()))
                context->scheduleSceneUpdate();
    }

    void GraphContextManager::handleModuleOutputPortNameChanged(Module* m, const u32 net)
    {
        Q_UNUSED(net);
        for (GraphContext* context : mContextTableModel->list())
            if (context->modules().contains(m->get_id()))
                context->scheduleSceneUpdate();
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
        return new StandardGraphLayouter(context);
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

    void GraphContextManager::restoreFromFile()
    {
        QString filename = FileManager::get_instance()->fileName();
        if (filename.isEmpty()) return;
        QFile jsFile(filename + "v");
        if (!jsFile.open(QIODevice::ReadOnly)) return;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsFile.readAll());
        const QJsonObject& json = jsonDoc.object();
        if (json.contains("views") && json["views"].isArray())
        {
            QJsonArray jsonViews = json["views"].toArray();
            int nviews = jsonViews.size();
            for (int iview=0; iview<nviews; iview++)
            {
                QJsonObject jsonView = jsonViews.at(iview).toObject();
                if (!jsonView.contains("id") || !jsonView["id"].isDouble()) continue;
                u32 viewId = jsonView["id"].toInt();
                if (!jsonView.contains("name") || !jsonView["name"].isString()) continue;
                QString viewName = jsonView["name"].toString();
                if (viewId > mMaxContextId) mMaxContextId = viewId;
                GraphContext* context = new GraphContext(viewId, viewName);
                context->setLayouter(getDefaultLayouter(context));
                context->setShader(getDefaultShader(context));
                context->readFromFile(jsonView);

                mContextTableModel->beginInsertContext(context);
                mContextTableModel->addContext(context);
                mContextTableModel->endInsertContext();

                Q_EMIT contextCreated(context);
                context->setDirty(false);
            }
        }
    }

    void GraphContextManager::handleSaveTriggered()
    {
        QString filename = FileManager::get_instance()->fileName();
        if (filename.isEmpty()) return;
        bool needToSave = false;
        for (GraphContext* context : mContextTableModel->list())
            if (context->id() > 1)
            {
                needToSave = true;
                break;
            }
        if (!needToSave) return;
        QFile jsFile(filename + "v");
        if (!jsFile.open(QIODevice::WriteOnly)) return;

        QJsonObject json;
        QJsonArray jsonViews;
        for (GraphContext* context : mContextTableModel->list())
            if (context->id() > 1)
            {
                QJsonObject jsonView;
                context->writeToFile(jsonView);
                jsonViews.append(jsonView);
            }
        json["views"] = jsonViews;
        jsFile.write(QJsonDocument(json).toJson(QJsonDocument::Compact));
    }
}
