#include "plugin_management/scheduled_plugin_item.h"

#include "gui_utils/graphics.h"

#include <QApplication>
#include <QDrag>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QStyle>

namespace hal
{
    bool scheduled_plugin_item::s_drag_in_progress = false;
    QPoint scheduled_plugin_item::s_drag_start_position;

    scheduled_plugin_item::scheduled_plugin_item(const QString& name, QWidget* parent) : QFrame(parent), m_layout(new QHBoxLayout()), m_label(new QLabel()), m_hover(false)
    {
        m_name = name;

        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);

        m_label->setObjectName("name-label");
        m_label->setText(name);

        setLayout(m_layout);
        m_layout->addWidget(m_label);

        repolish();
    }

    void scheduled_plugin_item::enterEvent(QEvent* event)
    {
        if (event->type() == QEvent::Enter)
        {
            m_hover = true;
            repolish();
        }
    }

    void scheduled_plugin_item::leaveEvent(QEvent* event)
    {
        if (event->type() == QEvent::Leave)
        {
            if (s_drag_in_progress)
                exec_drag();

            m_hover = false;
            repolish();
        }
    }

    void scheduled_plugin_item::mouseMoveEvent(QMouseEvent* event)
    {
        if (!s_drag_in_progress)
            return;
        if (!(event->buttons() & Qt::LeftButton))
            return;
        if ((event->pos() - s_drag_start_position).manhattanLength() < QApplication::startDragDistance())
            return;

        exec_drag();
    }

    void scheduled_plugin_item::mousePressEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::LeftButton)
        {
            s_drag_in_progress    = true;
            s_drag_start_position = event->pos();
        }

        event->accept();
        Q_EMIT clicked(this);
    }

    void scheduled_plugin_item::mouseReleaseEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::LeftButton)
            s_drag_in_progress = false;

        event->accept();
    }

    void scheduled_plugin_item::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        s->unpolish(m_label);
        s->polish(m_label);
    }

    void scheduled_plugin_item::exec_drag()
    {
        s_drag_in_progress   = false;
        QDrag* drag          = new QDrag(this);
        QMimeData* mime_data = new QMimeData();
        mime_data->setData("hal/plugin_name", m_label->text().toUtf8());
        mime_data->setData("hal/item_height", QString::number(height()).toUtf8());
        drag->setMimeData(mime_data);
        drag->setPixmap(grab());
        drag->setHotSpot(QPoint(drag->pixmap().width() / 2, drag->pixmap().height() / 2));

        m_hover = false;
        m_label->setText(m_name);
        repolish();

        Q_EMIT drag_started(this);
        Qt::DropAction dropAction = drag->exec(Qt::MoveAction);

        if (!(dropAction & Qt::MoveAction))
            Q_EMIT removed(this);
    }

    bool scheduled_plugin_item::hover()
    {
        return m_hover;
    }

    QString scheduled_plugin_item::icon_path()
    {
        return m_icon_path;
    }

    QString scheduled_plugin_item::icon_style()
    {
        return m_icon_style;
    }

    QString scheduled_plugin_item::name()
    {
        return m_label->text();
    }

    void scheduled_plugin_item::set_hover_active(bool active)
    {
        m_hover = active;
    }

    void scheduled_plugin_item::set_icon_path(const QString& path)
    {
        m_icon_path = path;
    }

    void scheduled_plugin_item::set_icon_style(const QString& style)
    {
        m_icon_style = style;
    }
}
