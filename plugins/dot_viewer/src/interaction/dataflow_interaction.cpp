#include "dot_viewer/interaction/dataflow_interaction.h"
#include "gui/gui_globals.h"
#include "gui/gui_api/gui_api.h"
#include <QRegularExpression>
#include <QGraphicsView>>
#include "QGVCore/QGVNode.h"
#include "QGVCore/QGVEdge.h"
#include <QMenu>

namespace hal {
    DataflowInteractionRegistration DataflowInteractionRegistration::sRegistration;

    QGVInteraction* constructDataflowInteraction(QGVScene* parent)
    {
        return new DataflowInteraction(parent);
    }

    DataflowInteractionRegistration::DataflowInteractionRegistration()
    {
        QGVInteraction::registerConstructorForPlugin("dataflow", constructDataflowInteraction);
    }

    DataflowInteraction::DataflowInteraction(QGVScene *parent)
        : QGVInteraction(parent), mScene(parent)
    {
        connect(gSelectionRelay, &SelectionRelay::selectionChanged, this, &DataflowInteraction::handleHALSelectionChanged);
        connect(parent, &QGVScene::edgeContextMenu, this, &DataflowInteraction::handleEdgeContextMenu);
        connect(parent, &QGraphicsScene::selectionChanged, this, &DataflowInteraction::handleQGVSelectionChanged);
    }

    void DataflowInteraction::registerEdge(QGVEdge* edge)
    {
        Q_UNUSED(edge);
    }

    void DataflowInteraction::registerNode(QGVNode* node)
    {
        QRegularExpression reDot("(\\d+) bit \\(id (\\d+)\\)");
        QRegularExpressionMatch mDot = reDot.match(node->label());
        if (mDot.hasMatch())
        {
            QRegularExpression reDana("DANA_module_(\\d+)");
            for (const Module* m : gNetlist->get_modules())
            {
                QRegularExpressionMatch mDana = reDana.match(QString::fromStdString(m->get_name()));
                if (mDana.hasMatch() && mDana.captured(1) == mDot.captured(2))
                {
                    mModuleHash[m->get_id()] = node;
                    mNodeHash[node] = m->get_id();
                }
            }
        }
    }

    void DataflowInteraction::handleQGVSelectionChanged()
    {
        if (mDisableHandler) return;
        mDisableHandler = true;
        std::vector<u32> modIds;
        for (QGraphicsItem* it : mScene->selectedItems())
        {
            QGVNode* node = dynamic_cast<QGVNode*>(it);
            if (!node) continue;
            u32 modId = mNodeHash.value(node,0);
            if (!modId) continue;
            modIds.push_back(modId);
        }
        gGuiApi->selectModule(modIds,true,true);
        mDisableHandler = false;
    }

    void DataflowInteraction::handleEdgeContextMenu(QGVEdge* edge)
    {
        if (!edge->headNode() || !edge->tailNode() || edge->headNode() == edge->tailNode())
            return;

        u32 tailModuleId = mNodeHash.value(edge->tailNode(),0);
        u32 headModuleId = mNodeHash.value(edge->headNode(),0);
        if (!tailModuleId || !headModuleId || tailModuleId == headModuleId) return;
        mDisableHandler = true;
        QMenu menu;
        QAction* act = menu.addAction(QString("Isolate path from mod_%1 to mod_%2 in new view").arg(tailModuleId).arg(headModuleId));
        connect(act, &QAction::triggered, this, [tailModuleId,headModuleId](){GuiApiClasses::View::isolateModuleToModulePathInNewView(tailModuleId,headModuleId);});
        menu.exec(QCursor::pos());
        mDisableHandler = false;
    }

    void DataflowInteraction::handleHALSelectionChanged(void* sender)
    {
        Q_UNUSED(sender);
        if (!mScene) return;

        if (mDisableHandler) return;
        mDisableHandler = true;

        mScene->clearSelection();
        for (u32 modId : gSelectionRelay->selectedModulesList())
        {
            QGVNode* node = mModuleHash.value(modId, nullptr);
            if (node)
            {
                node->setSelected(true);
                for (QGraphicsView* gv : mScene->views())
                    gv->ensureVisible(node);
            }
        }
        mDisableHandler = false;
    }
}
