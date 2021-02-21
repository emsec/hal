#include "gui/graph_tab_widget/graph_tab_widget.h"

#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui/graph_widget/graph_graphics_view.h"
#include "gui/settings/settings_items/settings_item_dropdown.h"
#include "gui/settings/settings_items/settings_item_keybind.h"

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
        
        mSettingGridType = new SettingsItemDropdown(
            "Background Grid",
            "graph_view/grid_type",
            GraphicsScene::GridType::None,
            "Graph View",
            "Specifies the grid pattern in the background of the Graph View scene"
        );
        mSettingGridType->setValueNames<GraphicsScene::GridType>();

        mSettingDragModifier = new SettingsItemDropdown(
            "Move/Swap Modifier",
            "graph_view/drag_mode_modifier",
            KeyboardModifier::Alt,
            "Graph View",
            "Specifies the key which can be pressed to switch the position of two Module/Gates in the Graph View while dragging."
       );
        mSettingDragModifier->setValueNames<KeyboardModifier>();

        mSettingPanModifier = new SettingsItemDropdown(
            "Pan Scene Modifier",
            "graph_view/pan_modifier",
            KeyboardModifier::Shift,
            "Graph View",
            "Specifies the key which can be pressed to pan the scene in the Graph View while left clicking."
        );
        mSettingPanModifier->setValueNames<KeyboardModifier>();

        //cant use the qt enum Qt::KeyboardModifer, therefore a map as santas little helper
        mKeyModifierMap = QMap<KeyboardModifier, Qt::KeyboardModifier>();
        mKeyModifierMap.insert(KeyboardModifier::Alt, Qt::KeyboardModifier::AltModifier);
        mKeyModifierMap.insert(KeyboardModifier::Ctrl, Qt::KeyboardModifier::ControlModifier);
        mKeyModifierMap.insert(KeyboardModifier::Shift, Qt::KeyboardModifier::ShiftModifier);

        mSettingZoomIn = new SettingsItemKeybind(
            "Graph View Zoom In",
            "keybind/graph_zoom_in",
            QKeySequence("Ctrl++"),
            "Keybindings: Graph",
            "Keybind for zooming in in the Graph View."
        );

        mSettingZoomOut = new SettingsItemKeybind(
            "Graph View Zoom Out",
            "keybind/graph_zoom_out",
            QKeySequence("Ctrl+-"),
            "Keybindings: Graph",
            "Keybind for zooming out in the Graph View."
        );
    }

    QList<QShortcut *> GraphTabWidget::createShortcuts()
    {
        QShortcut* zoomInShortcut = new QShortcut(mSettingZoomIn->value().toString(), this);
        QShortcut* zoomOutShortcut = new QShortcut(mSettingZoomOut->value().toString(), this);

        connect(zoomInShortcut, &QShortcut::activated, this, &GraphTabWidget::zoomInShortcut);
        connect(zoomOutShortcut, &QShortcut::activated, this, &GraphTabWidget::zoomOutShortcut);

        connect(mSettingZoomIn, &SettingsItemKeybind::keySequenceChanged, zoomInShortcut, &QShortcut::setKey);
        connect(mSettingZoomOut, &SettingsItemKeybind::keySequenceChanged, zoomOutShortcut, &QShortcut::setKey);

        QList<QShortcut*> list;
        list.append(zoomInShortcut);
        list.append(zoomOutShortcut);

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

        new_graph_widget->view()->setGridType((GraphicsScene::GridType(mSettingGridType->value().toInt())));
        new_graph_widget->view()->setDragModifier(mKeyModifierMap.value((KeyboardModifier)mSettingDragModifier->value().toInt()));
        new_graph_widget->view()->setPanModifier(mKeyModifierMap.value((KeyboardModifier)mSettingPanModifier->value().toInt()));

        connect(mSettingGridType, &SettingsItemDropdown::intChanged, new_graph_widget, [new_graph_widget](int value){
            new_graph_widget->view()->setGridType((GraphicsScene::GridType(value)));
        });

        connect(mSettingDragModifier, &SettingsItemDropdown::intChanged, new_graph_widget, [new_graph_widget, this](int value){
            new_graph_widget->view()->setDragModifier(mKeyModifierMap.value((KeyboardModifier)value));
        });

        connect(mSettingPanModifier, &SettingsItemDropdown::intChanged, new_graph_widget, [new_graph_widget, this](int value){
            new_graph_widget->view()->setPanModifier(mKeyModifierMap.value((KeyboardModifier)value));
        });

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
