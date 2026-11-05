#include "dot_viewer/interaction/clock_tree_extractor_interaction.h"
#include "gui/gui_globals.h"
#include "gui/gui_api/gui_api.h"
#include <QRegularExpression>
#include <QGraphicsView>>
#include "QGVCore/QGVNode.h"
#include "QGVCore/QGVEdge.h"
#include <QMenu>

namespace hal {
    ClockTreeExtractorInteractionRegistration ClockTreeExtractorInteractionRegistration::sRegistration;

    QGVInteraction* constructClockTreeExtractorInteraction(QGVScene* parent)
    {
        return new ClockTreeExtractorInteraction(parent);
    }

    ClockTreeExtractorInteractionRegistration::ClockTreeExtractorInteractionRegistration()
    {
        QGVInteraction::registerConstructorForPlugin("clock_tree_extractor", constructClockTreeExtractorInteraction);
    }

    ClockTreeExtractorInteraction::ClockTreeExtractorInteraction(QGVScene *parent)
        : QGVInteraction(parent), mScene(parent)
    {
        connect(gSelectionRelay, &SelectionRelay::selectionChanged, this, &ClockTreeExtractorInteraction::handleHALSelectionChanged);
        connect(parent, &QGraphicsScene::selectionChanged, this, &ClockTreeExtractorInteraction::handleQGVSelectionChanged);
    }

    void ClockTreeExtractorInteraction::registerEdge(QGVEdge* edge)
    {
        Q_UNUSED(edge);
    }

    void ClockTreeExtractorInteraction::registerNode(QGVNode* node)
    {
        bool ok = false;

        if (node->getShape() == "circle")
        {
            // global Input
            bool assigned = false;
            for (Net* n : gNetlist->get_global_input_nets())
            {
                if (n->get_name() == node->label().trimmed().toStdString())
                {
                    mNetHash[n->get_id()] = node;
                    mGlobalInputHash[node] = n->get_id();
                    assigned = true;
                    break;
                }
            }
            if (!assigned)
                log_warning("dot_viewer", "Cannot assign circle with label='{}' to global input.", node->label().toStdString());
            return;
        }
        u32 gateId = node->label().trimmed().toInt(&ok);
        if (!ok)
        {
            log_warning("dot_viewer", "Cannot parse gate ID from string '{}'.", node->label().toStdString());
            return;
        }

        Gate* g = gNetlist->get_gate_by_id(gateId);
        if (!g)
        {
            log_warning("dot_viewer", "Gate with ID={} not found in netlist.", gateId);
            return;
        }

        mGateHash[gateId] = node;
        mNodeHash[node] = gateId;
    }

    void ClockTreeExtractorInteraction::handleQGVSelectionChanged()
    {
        if (mDisableHandler) return;
        mDisableHandler = true;
        std::vector<u32> gatIds;
        std::vector<u32> netIds;
        for (QGraphicsItem* it : mScene->selectedItems())
        {
            QGVNode* node = dynamic_cast<QGVNode*>(it);
            if (!node) continue;
            if (node->getShape() == "circle")
            {
                u32 netId = mGlobalInputHash.value(node,0);
                if (netId) netIds.push_back(netId);
            }
            else
            {
                u32 gatId = mNodeHash.value(node,0);
                if (gatId) gatIds.push_back(gatId);
            }
        }
        gGuiApi->selectGate(gatIds,true,true);
        if (!netIds.empty())
            gGuiApi->selectNet(netIds,false,true);
        mDisableHandler = false;
    }

    void ClockTreeExtractorInteraction::handleHALSelectionChanged(void* sender)
    {
        Q_UNUSED(sender);
        if (!mScene) return;

        if (mDisableHandler) return;
        mDisableHandler = true;

        mScene->clearSelection();
        for (u32 gatId : gSelectionRelay->selectedGatesList())
        {
            QGVNode* node = mGateHash.value(gatId, nullptr);
            if (node)
            {
                node->setSelected(true);
                for (QGraphicsView* gv : mScene->views())
                    gv->ensureVisible(node);
            }
        }

        for (u32 netId : gSelectionRelay->selectedNetsList())
        {
            QGVNode* node = mNetHash.value(netId, nullptr);
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
