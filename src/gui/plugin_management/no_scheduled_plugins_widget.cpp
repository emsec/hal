#include "plugin_management/no_scheduled_plugins_widget.h"

#include "file_manager/file_manager.h"
#include "gui_utils/graphics.h"

#include <QDragEnterEvent>
#include <QLabel>
#include <QMimeData>
#include <QStyle>
#include <QVBoxLayout>
namespace hal{
no_scheduled_plugins_widget::no_scheduled_plugins_widget(QWidget* parent) : QFrame(parent), m_layout(new QVBoxLayout()), m_text_label(new QLabel()), m_icon_label(new QLabel()), m_drag_active(false)
{
    setAcceptDrops(true);

    m_text_label->setObjectName("text-label");
    m_text_label->setText("Drag & drop plugins here to create a new schedule, select a scheduled plugin to edit its arguments");
    m_text_label->setWordWrap(true);

    m_icon_label->setObjectName("icon-label");
    m_icon_label->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_icon_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setLayout(m_layout);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addWidget(m_text_label);
    m_layout->addWidget(m_icon_label);
}

void no_scheduled_plugins_widget::dragEnterEvent(QDragEnterEvent* event)
{
    const QMimeData* mime_data = event->mimeData();
    QString name               = QString(mime_data->data("hal/plugin_name"));
    if (name.isEmpty())
        return;

    event->acceptProposedAction();
    m_drag_active = true;
    repolish();
}

void no_scheduled_plugins_widget::dragLeaveEvent(QDragLeaveEvent* event)
{
    Q_UNUSED(event)

    m_drag_active = false;
    repolish();
}

void no_scheduled_plugins_widget::dropEvent(QDropEvent* event)
{
    const QMimeData* mime_data = event->mimeData();
    QString name               = QString(mime_data->data("hal/plugin_name"));
    if (name.isEmpty())
        return;

    event->acceptProposedAction();
    m_drag_active = false;
    repolish();

    hide();
    Q_EMIT append_plugin(name);
}

bool no_scheduled_plugins_widget::drag_active()
{
    return m_drag_active;
}

QString no_scheduled_plugins_widget::icon_style()
{
    return m_icon_style;
}

QString no_scheduled_plugins_widget::icon_path()
{
    return m_icon_path;
}

void no_scheduled_plugins_widget::set_icon_style(const QString& style)
{
    m_icon_style = style;
}

void no_scheduled_plugins_widget::set_icon_path(const QString& path)
{
    m_icon_path = path;
}

void no_scheduled_plugins_widget::repolish()
{
    QStyle* s = style();

    s->unpolish(this);
    s->polish(this);

    if (!m_icon_path.isEmpty())
        m_icon_label->setPixmap(gui_utility::get_styled_svg_icon(m_icon_style, m_icon_path).pixmap(QSize(120, 120)));
}
}
