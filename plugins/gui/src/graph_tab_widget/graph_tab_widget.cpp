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
#include <QCursor>
#include <QPixmap>

namespace hal
{
    SettingsItemDropdown* GraphTabWidget::sSettingGridType;
    SettingsItemDropdown* GraphTabWidget::sSettingDragModifier;
    SettingsItemDropdown* GraphTabWidget::sSettingPanModifier;

    SettingsItemKeybind* GraphTabWidget::sSettingZoomIn;
    SettingsItemKeybind* GraphTabWidget::sSettingZoomOut;

    bool GraphTabWidget::sSettingsInitialized = initSettings();

    bool GraphTabWidget::initSettings()
    {
        sSettingGridType = new SettingsItemDropdown(
            "Background Grid",
            "graph_view/grid_type",
            GraphicsScene::GridType::None,
            "Appearance:Graph View",
            "Specifies the grid pattern in the background of the Graph View scene"
        );
        sSettingGridType->setValueNames<GraphicsScene::GridType>();

        sSettingDragModifier = new SettingsItemDropdown(
            "Move/Swap Modifier",
            "graph_view/drag_mode_modifier",
            KeyboardModifier::Alt,
            "Graph View",
            "Specifies the key which can be pressed to switch the position of two Module/Gates in the Graph View while dragging."
        );
        sSettingDragModifier->setValueNames<KeyboardModifier>();

        sSettingPanModifier = new SettingsItemDropdown(
            "Pan Scene Modifier",
            "graph_view/move_modifier",
            KeyboardModifier::Shift,
            "Graph View",
            "Specifies the key which can be pressed to pan the scene in the Graph View while left clicking."
        );
        sSettingPanModifier->setValueNames<KeyboardModifier>();

        sSettingZoomIn = new SettingsItemKeybind(
            "Graph View Zoom In",
            "keybinds/graph_view_zoom_in",
            QKeySequence("Ctrl++"),
            "Keybindings:Graph",
            "Keybind for zooming in in the Graph View."
        );

        sSettingZoomOut = new SettingsItemKeybind(
            "Graph View Zoom Out",
            "keybinds/graph_view_zoom_out",
            QKeySequence("Ctrl+-"),
            "Keybindings:Graph",
            "Keybind for zooming out in the Graph View."
        );

        return true;
    }

    GraphTabWidget::GraphTabWidget(QWidget* parent) : ContentWidget("Graph-Views", parent),
        mTabWidget(new QTabWidget()), mLayout(new QVBoxLayout()), mZoomFactor(1.2),
        mSelectCursor(Select)
    {
        mContentLayout->addWidget(mTabWidget);
        mTabWidget->setTabsClosable(true);
        mTabWidget->setMovable(true);
        mTabWidget->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(mTabWidget, &QTabWidget::customContextMenuRequested, this, &GraphTabWidget::handleCustomContextMenuRequested);
        connect(mTabWidget, &QTabWidget::tabCloseRequested, this, &GraphTabWidget::handleTabCloseRequested);
        connect(mTabWidget, &QTabWidget::currentChanged, this, &GraphTabWidget::handleTabChanged);
        connect(gGraphContextManager, &GraphContextManager::contextCreated, this, &GraphTabWidget::handleContextCreated);
        connect(gGraphContextManager, &GraphContextManager::contextRenamed, this, &GraphTabWidget::handleContextRenamed);
        connect(gGraphContextManager, &GraphContextManager::deletingContext, this, &GraphTabWidget::handleContextRemoved);
        connect(gGuiApi, &GuiApi::navigationRequested, this, &GraphTabWidget::ensureSelectionVisible);
        

        //cant use the qt enum Qt::KeyboardModifer, therefore a map as santas little helper
        mKeyModifierMap = QMap<KeyboardModifier, Qt::KeyboardModifier>();
        mKeyModifierMap.insert(KeyboardModifier::Alt, Qt::KeyboardModifier::AltModifier);
        mKeyModifierMap.insert(KeyboardModifier::Ctrl, Qt::KeyboardModifier::ControlModifier);
        mKeyModifierMap.insert(KeyboardModifier::Shift, Qt::KeyboardModifier::ShiftModifier);
    }

    QList<QShortcut *> GraphTabWidget::createShortcuts()
    {
        QShortcut* zoomInShortcut = new QShortcut(sSettingZoomIn->value().toString(), this);
        QShortcut* zoomOutShortcut = new QShortcut(sSettingZoomOut->value().toString(), this);

        connect(zoomInShortcut, &QShortcut::activated, this, &GraphTabWidget::zoomInShortcut);
        connect(zoomOutShortcut, &QShortcut::activated, this, &GraphTabWidget::zoomOutShortcut);

        connect(sSettingZoomIn, &SettingsItemKeybind::keySequenceChanged, zoomInShortcut, &QShortcut::setKey);
        connect(sSettingZoomOut, &SettingsItemKeybind::keySequenceChanged, zoomOutShortcut, &QShortcut::setKey);

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

        new_graph_widget->view()->setGridType((GraphicsScene::GridType(sSettingGridType->value().toInt())));
        new_graph_widget->view()->setDragModifier(mKeyModifierMap.value((KeyboardModifier)sSettingDragModifier->value().toInt()));
        new_graph_widget->view()->setPanModifier(mKeyModifierMap.value((KeyboardModifier)sSettingPanModifier->value().toInt()));

        connect(sSettingGridType, &SettingsItemDropdown::intChanged, new_graph_widget, [new_graph_widget](int value){
            new_graph_widget->view()->setGridType((GraphicsScene::GridType(value)));
        });

        connect(sSettingDragModifier, &SettingsItemDropdown::intChanged, new_graph_widget, [new_graph_widget, this](int value){
            new_graph_widget->view()->setDragModifier(mKeyModifierMap.value((KeyboardModifier)value));
        });

        connect(sSettingPanModifier, &SettingsItemDropdown::intChanged, new_graph_widget, [new_graph_widget, this](int value){
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

    void GraphTabWidget::handleCustomContextMenuRequested(const QPoint &pos)
    {
        int index = mTabWidget->tabBar()->tabAt(pos);

        if (index == -1)
            return;

        QMenu contextMenu("Context menu", this);

        contextMenu.addAction("Close", [this, index](){
            handleTabCloseRequested(index);
        });

        contextMenu.addSeparator();

        contextMenu.addAction("Close all", [this, index](){
            handleCloseAllTabs();
        });

        contextMenu.addAction("Close all others", [this, index](){
            handleCloseTabsToRight(index);
            handleCloseTabsToLeft(index);
        });

        contextMenu.addAction("Close all right", [this, index](){
            handleCloseTabsToRight(index);
        });

        contextMenu.addAction("Close all left", [this, index](){
            handleCloseTabsToLeft(index);
        });

        contextMenu.exec(mapToGlobal(pos));

    }

    void GraphTabWidget::handleCloseTabsToRight(int index)
    {
        // Close last tab until tab at index is the last tab
        while (index != mTabWidget->count()-1)
            handleTabCloseRequested(mTabWidget->count()-1);
    }

    void GraphTabWidget::handleCloseTabsToLeft(int index)
    {
        // Close first tab until tab at index is the first tab
        while (index != 0)
        {
            handleTabCloseRequested(0);
            index-=1;
        }
    }

    void GraphTabWidget::handleCloseAllTabs()
    {
        int count = mTabWidget->count();
        for (int i = 0; i < count; i++)
        {
            handleTabCloseRequested(0);
        }
    }

    void GraphTabWidget::setSelectCursor(int icurs)
    {
        mSelectCursor = (GraphCursor) icurs;
        int n = mTabWidget->count();
        if (mSelectCursor == Select)
        {
            for (int i=0; i<n; i++)
                mTabWidget->widget(i)->unsetCursor();
        }
        else
        {
            QCursor gcurs(mSelectCursor == PickModule
                          ? QPixmap(":/icons/module_cursor","PNG")
                          : QPixmap(":/icons/gate_cursor","PNG"));
            for (int i=0; i<n; i++)
                mTabWidget->widget(i)->setCursor(gcurs);
        }
    }
}
