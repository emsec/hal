#include "gui/graph_tab_widget/graph_tab_widget.h"

#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui/graph_widget/graph_graphics_view.h"

#include "gui/gui_globals.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QShortcut>
#include <QKeySequence>

namespace hal
{
    GraphTabWidget::GraphTabWidget(QWidget* parent) : ContentWidget("Graph-Views", parent), mTabWidget(new QTabWidget()), mLayout(new QVBoxLayout()), mZoomFactor(1.2)
    {
        mContentLayout->addWidget(mTabWidget);
        mTabWidget->setTabsClosable(true);
        mTabWidget->setMovable(true);

        connect(mTabWidget, &QTabWidget::tabCloseRequested, this, &GraphTabWidget::handleTabCloseRequested);
        connect(mTabWidget, &QTabWidget::currentChanged, this, &GraphTabWidget::handleTabChanged);
        connect(gGraphContextManager, &GraphContextManager::contextCreated, this, &GraphTabWidget::handleContextCreated);
        connect(gGraphContextManager, &GraphContextManager::contextRenamed, this, &GraphTabWidget::handleContextRenamed);
        connect(gGraphContextManager, &GraphContextManager::deletingContext, this, &GraphTabWidget::handleContextRemoved);
        connect(gGuiApi, &GuiApi::navigationRequested, this, &GraphTabWidget::ensureSelectionVisible);
    }

    QList<QShortcut *> GraphTabWidget::createShortcuts()
    {
        QShortcut* zoom_in_sc = gKeybindManager->makeShortcut(this, "keybinds/graph_view_zoom_in");
        connect(zoom_in_sc, &QShortcut::activated, this, &GraphTabWidget::zoomInShortcut);

        QShortcut* zoom_out_sc = gKeybindManager->makeShortcut(this, "keybinds/graph_view_zoom_out");
        connect(zoom_out_sc, &QShortcut::activated, this, &GraphTabWidget::zoomOutShortcut);

        QList<QShortcut*> list;
        list.append(zoom_in_sc);
        list.append(zoom_out_sc);

        return list;
    }

    int GraphTabWidget::addTab(QWidget* tab, QString name)
    {
        int tab_index = mTabWidget->addTab(tab, name);
        return tab_index;
    }

    void GraphTabWidget::handleTabChanged(int index)
    {
        auto w = dynamic_cast<GraphWidget*>(mTabWidget->widget(index));
        if (w)
        {
            auto ctx = w->getContext();
            gContentManager->getContextManagerWidget()->selectViewContext(ctx);
        }
    }

    void GraphTabWidget::ensureSelectionVisible()
    {
        GraphWidget* currentGraphWidget = dynamic_cast<GraphWidget*>(mTabWidget->currentWidget());
        if(currentGraphWidget)
            currentGraphWidget->ensureSelectionVisible();
    }

    void GraphTabWidget::handleTabCloseRequested(int index)
    {
        mTabWidget->removeTab(index);

        //right way to do it??
        //GraphWidget* graph_wid = dynamic_cast<GraphWidget*>(mTabWidget->widget(index));
        //GraphContext* dyn_con = gGraphContextManager->get_context_by_name(mTabWidget->tabText(index));
        //dyn_con->unsubscribe(graph_wid);
    }

    void GraphTabWidget::showContext(GraphContext* context)
    {
        auto index = getContextTabIndex(context);
        if (index != -1)
        {
            mTabWidget->setCurrentIndex(index);
            mTabWidget->widget(index)->setFocus();
            return;
        }

        addGraphWidgetTab(context);
    }

    void GraphTabWidget::handleContextCreated(GraphContext* context)
    {
        addGraphWidgetTab(context);
    }

    void GraphTabWidget::handleContextRenamed(GraphContext* context)
    {
        mTabWidget->setTabText(getContextTabIndex(context), context->name());
    }

    void GraphTabWidget::handleContextRemoved(GraphContext* context)
    {
        handleTabCloseRequested(getContextTabIndex(context));
    }

    void GraphTabWidget::handleGateFocus(u32 gateId)
    {
        GraphWidget* currentGraphWidget = dynamic_cast<GraphWidget*>(mTabWidget->currentWidget());
        if(currentGraphWidget)
            currentGraphWidget->focusGate(gateId);
    }

    void GraphTabWidget::handleNetFocus(u32 netId)
    {
        GraphWidget* currentGraphWidget = dynamic_cast<GraphWidget*>(mTabWidget->currentWidget());
        if(currentGraphWidget)
            currentGraphWidget->focusNet(netId);
    }

    void GraphTabWidget::handleModuleFocus(u32 moduleId)
    {
        GraphWidget* currentGraphWidget = dynamic_cast<GraphWidget*>(mTabWidget->currentWidget());
        if(currentGraphWidget)
            currentGraphWidget->focusModule(moduleId);
    }

    void GraphTabWidget::addGraphWidgetTab(GraphContext* context)
    {
        GraphWidget* new_graph_widget = new GraphWidget(context);
        //mContextWidgetMap.insert(context, new_graph_widget);

        int tab_index = addTab(new_graph_widget, context->name());
        mTabWidget->setCurrentIndex(tab_index);
        mTabWidget->widget(tab_index)->setFocus();
        context->scheduleSceneUpdate();
    }

    void GraphTabWidget::zoomInShortcut()
    {
        GraphWidget* w = dynamic_cast<GraphWidget*>(mTabWidget->currentWidget());
        if(w)
            w->view()->viewportCenterZoom(mZoomFactor);
    }

    void GraphTabWidget::zoomOutShortcut()
    {
        GraphWidget* w = dynamic_cast<GraphWidget*>(mTabWidget->currentWidget());
        if(w)
            w->view()->viewportCenterZoom(1.0 / mZoomFactor);

    }

    int GraphTabWidget::getContextTabIndex(GraphContext* context) const
    {
        for (int i = 0; i < mTabWidget->count(); i++)
        {
            if (auto p = dynamic_cast<GraphWidget*>(mTabWidget->widget(i)))
            {
                if (p->getContext() == context)
                {
                    return i;
                }
            }
        }
        return -1;
    }
}
