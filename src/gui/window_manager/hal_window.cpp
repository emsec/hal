#include "window_manager/hal_window.h"

#include "gui_globals.h"
#include "overlay/overlay.h"
#include "window_manager/hal_window_layout_container.h"
#include "window_manager/hal_window_toolbar.h"
#include "window_manager/workspace.h"

#include <QAction>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QStyle>

#include <QGraphicsBlurEffect>
namespace hal{
hal_window::hal_window(QWidget* parent) : QFrame(parent),
    m_outer_layout(new QVBoxLayout(this)),
    m_layout_container(new hal_window_layout_container(this)),
    m_inner_layout(new QVBoxLayout(m_layout_container)),
    m_toolbar(new hal_window_toolbar(this)),
    m_workspace(new workspace(this)),
    m_overlay(nullptr),
    m_effect(nullptr)
{
//    m_layout_container->setGeometry(geometry());

    m_outer_layout->setContentsMargins(0, 0, 0, 0);
    m_outer_layout->setSpacing(0);

    m_inner_layout->setContentsMargins(0, 0, 0, 0);
    m_inner_layout->setSpacing(0);

    m_toolbar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_workspace->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_inner_layout->addWidget(m_toolbar);
    m_inner_layout->addWidget(m_workspace);
    m_outer_layout->addWidget(m_layout_container);

    // OPTIMIZATION: MANAGE SUB WIDGETS MANUALLY TO AVOID (MINIMAL) LAYOUT OVERHEAD ?
}

void hal_window::lock()
{
    if (!m_overlay)
    {
        m_layout_container->setEnabled(false);
//        m_toolbar->setEnabled(false);
//        m_workspace->setEnabled(false);

        m_overlay = new overlay(this); // DEBUG CODE, USE FACTORY AND STYLESHEETS ?
        m_effect = new QGraphicsBlurEffect();
        //m_effect->setBlurHints(QGraphicsBlurEffect::QualityHint);
        m_layout_container->setGraphicsEffect(m_effect);

//        QGraphicsBlurEffect* effect = new QGraphicsBlurEffect();
//        effect->setBlurHints(QGraphicsBlurEffect::QualityHint);
//        m_toolbar->setGraphicsEffect(effect);

//        QGraphicsBlurEffect* effect2 = new QGraphicsBlurEffect();
//        effect2->setBlurHints(QGraphicsBlurEffect::QualityHint);
//        m_workspace->setGraphicsEffect(effect2);

        m_overlay->setParent(this);
        m_overlay->show();
        connect(m_overlay, &overlay::clicked, g_window_manager, &window_manager::handle_overlay_clicked);
        // FADE IN ???
    }
}

void hal_window::unlock()
{
    if (m_overlay)
    {
        m_layout_container->setEnabled(true);
//        m_toolbar->setEnabled(true);
//        m_workspace->setEnabled(true);

        // FADE OUT ???
        m_effect->deleteLater();
        m_overlay->deleteLater();
        m_overlay = nullptr;
    }
}

void hal_window::standard_view()
{
    m_workspace->show();
}

void hal_window::special_view(QWidget* widget)
{
    m_workspace->hide();
    m_inner_layout->addWidget(widget);
}

void hal_window::repolish()
{
    QStyle* s = style();

    s->unpolish(this);
    s->polish(this);

    m_toolbar->repolish();
    m_workspace->repolish();
    // REPOLISH CONTENT THROUGH CONTENT MANAGER

    //rearrange();
}

hal_window_toolbar* hal_window::get_toolbar()
{
    return m_toolbar;
}

overlay* hal_window::get_overlay()
{
    return m_overlay;
}

//void hal_window::paintEvent(QPaintEvent* event)
// {
//    Q_UNUSED(event)

//    // LEFT EMPTY INTENTIONALLY
// }

//bool hal_window::event(QEvent* event)
//{
//    if (event->type() == QEvent::WindowStateChange)
//    {
//        if (isMinimized())
//        {
//            //hide();
//            //event->accept();
//            event->ignore(); // TEST
//            // NOTIFIY WINDOW MANAGER
//            return true;
//        }
//    }

//    QWidget::event(event);
//}

void hal_window::closeEvent(QCloseEvent* event)
{
    g_window_manager->handle_window_close_request(this);
    event->ignore();
}

//void hal_window::changeEvent(QEvent* event)
//{
//    if (event->type() == QEvent::WindowStateChange)
//    {
//        if( windowState() == Qt::WindowMinimized )
//        {
//            event->accept();
//        }
//        else if( windowState() == Qt::WindowNoState )
//        {
//            event->accept();
//            // ???
//        }
//    }
//}

//void hal_window::resizeEvent(QResizeEvent* event)
//{
//    m_layout_container->resize(size());
//    event->accept(); // UNNECESSARY ?
//    //rearrange();
//}

//void hal_window::rearrange()
//{
//    // USE QT LAYOUT ?
//    // SMALL OVERHEAD, PROBABLY STILL SMARTER

//    QRect contents_rect = contentsRect();

//    int x = contents_rect.x();
//    int y = contents_rect.y();

//    int width = contents_rect.width();
//    int height = contents_rect. height();

//    int toolbar_height = m_toolbar->sizeHint().height();

//    m_toolbar->setGeometry(x, y, width, toolbar_height - 1);
//    //m_workspace->setGeometry(x, y + toolbar_height, width, height - toolbar_height);

//    //TEST
//    m_toolbar->show();
//    //m_workspace->hide();
//}
}
