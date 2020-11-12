#include "gui/window_manager/window.h"

#include "gui/gui_globals.h"
#include "gui/graphics_effects/graphics_effect_factory.h"
#include "gui/overlay/overlay.h"
#include "gui/window_manager/window_effect_layer.h"
#include "gui/window_manager/window_toolbar.h"
#include "gui/window_manager/workspace.h"

#include <QAction>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QStyle>

#include <cassert>

namespace hal
{
    Window::Window(QWidget* parent) : QFrame(parent),
        mWindowLayout(new QVBoxLayout(this)),
        mEffectLayer(new WindowEffectLayer(this)),
        mEffectLayerLayout(new QVBoxLayout(mEffectLayer)),
        mToolbar(nullptr),
        mToolbarExtension(new QFrame(this)),
        mWorkspace(new Workspace(this)),
        mOverlay(nullptr),
        mEffect(nullptr)
    {
        mWindowLayout->setContentsMargins(0, 0, 0, 0);
        mWindowLayout->setSpacing(0);

        mEffectLayerLayout->setContentsMargins(0, 0, 0, 0);
        mEffectLayerLayout->setSpacing(0);

        mToolbarExtension->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        mWorkspace->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        mEffectLayerLayout->addWidget(mToolbarExtension);
        mEffectLayerLayout->addWidget(mWorkspace);

        mWindowLayout->addWidget(mEffectLayer);
    }

    void Window::showToolbar(WindowToolbar* toolbar)
    {
        assert(toolbar);

        mEffectLayerLayout->insertWidget(0, toolbar);
    }

    void Window::hideToolbarExtension()
    {
        mToolbarExtension->hide();
    }

    void Window::showToolbarExtension()
    {
        mToolbarExtension->show();
    }

    void Window::showWorkspace()
    {
        mWorkspace->show();
    }

    void Window::showSpecialScreen(QWidget* widget)
    {
        assert(widget);

        mWorkspace->hide();
        mEffectLayerLayout->addWidget(widget);
        widget->show();
    }

    void Window::lock(Overlay* overlay)
    {
        assert(overlay);

        if (!mOverlay)
        {
            mEffectLayer->setEnabled(false);

            mEffect = GraphicsEffectFactory::getEffect();
            mEffectLayer->setGraphicsEffect(mEffect);

            mOverlay = overlay;
            mOverlay->setParent(this);
            mOverlay->show();
            connect(mOverlay, &Overlay::clicked, gWindowManager, &WindowManager::handleOverlayClicked);
        }
    }

    void Window::unlock()
    {
        if (mOverlay)
        {
            mEffectLayer->setEnabled(true);

            delete mOverlay;
            delete mEffect;

            mOverlay = nullptr;
            mEffect = nullptr;
        }
    }

    void Window::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        mToolbar->repolish();
        mWorkspace->repolish();

        // REPOLISH CONTENT THROUGH CONTENT MANAGER
    }

    void Window::closeEvent(QCloseEvent* event)
    {
        gWindowManager->handleWindowCloseRequest(this);
        event->ignore();
    }
}
