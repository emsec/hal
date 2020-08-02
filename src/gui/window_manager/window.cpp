#include "gui/window_manager/window.h"

#include "gui_globals.h"
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
        m_window_layout(new QVBoxLayout(this)),
        m_effect_layer(new WindowEffectLayer(this)),
        m_effect_layer_layout(new QVBoxLayout(m_effect_layer)),
        m_toolbar(nullptr),
        m_toolbar_extension(new QFrame(this)),
        m_workspace(new Workspace(this)),
        m_overlay(nullptr),
        m_effect(nullptr)
    {
        m_window_layout->setContentsMargins(0, 0, 0, 0);
        m_window_layout->setSpacing(0);

        m_effect_layer_layout->setContentsMargins(0, 0, 0, 0);
        m_effect_layer_layout->setSpacing(0);

        m_toolbar_extension->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        m_workspace->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        m_effect_layer_layout->addWidget(m_toolbar_extension);
        m_effect_layer_layout->addWidget(m_workspace);

        m_window_layout->addWidget(m_effect_layer);
    }

    void Window::show_toolbar(WindowToolbar* toolbar)
    {
        assert(toolbar);

        m_effect_layer_layout->insertWidget(0, toolbar);
    }

    void Window::hide_toolbar_extension()
    {
        m_toolbar_extension->hide();
    }

    void Window::show_toolbar_extension()
    {
        m_toolbar_extension->show();
    }

    void Window::show_workspace()
    {
        m_workspace->show();
    }

    void Window::show_special_screen(QWidget* widget)
    {
        assert(widget);

        m_workspace->hide();
        m_effect_layer_layout->addWidget(widget);
        widget->show();
    }

    void Window::lock(Overlay* overlay)
    {
        assert(overlay);

        if (!m_overlay)
        {
            m_effect_layer->setEnabled(false);

            m_effect = GraphicsEffectFactory::get_effect();
            m_effect_layer->setGraphicsEffect(m_effect);

            m_overlay = overlay;
            m_overlay->setParent(this);
            m_overlay->show();
            connect(m_overlay, &Overlay::clicked, g_window_manager, &WindowManager::handle_overlay_clicked);
        }
    }

    void Window::unlock()
    {
        if (m_overlay)
        {
            m_effect_layer->setEnabled(true);

            delete m_overlay;
            delete m_effect;

            m_overlay = nullptr;
            m_effect = nullptr;
        }
    }

    void Window::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        m_toolbar->repolish();
        m_workspace->repolish();

        // REPOLISH CONTENT THROUGH CONTENT MANAGER
    }

    void Window::closeEvent(QCloseEvent* event)
    {
        g_window_manager->handle_window_close_request(this);
        event->ignore();
    }
}
