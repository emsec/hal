#include "content_widget/content_widget.h"

#include "gui_utils/graphics.h"
#include "hal_content_anchor/hal_content_anchor.h"
#include "toolbar/toolbar.h"

#include <QShortcut>
#include <QStyle>
#include <QVBoxLayout>
namespace hal{
content_widget::content_widget(QString name, QWidget* parent) : hal_widget(parent), m_name(name), m_content_layout(new QVBoxLayout())
{
    m_content_layout->setContentsMargins(0, 0, 0, 0);
    m_content_layout->setSpacing(0);
    setMinimumSize(100, 100);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setWindowTitle(name);

    setLayout(m_content_layout);
    //debug code, delete later
    QIcon temp;
    temp.addFile(QStringLiteral(":/icons/start"), QSize(), QIcon::Normal, QIcon::Off);
    set_icon(temp);
}

void content_widget::remove()
{
    if (m_anchor)
    {
        m_anchor->remove(this);
        Q_EMIT removed();
    }
}

void content_widget::detach()
{
    if (m_anchor)
    {
        m_anchor->detach(this);
        Q_EMIT detached();
    }
}

void content_widget::reattach()
{
    if (m_anchor)
    {
        m_anchor->reattach(this);
        Q_EMIT reattached();
    }
}

void content_widget::open()
{
    if (m_anchor)
    {
        m_anchor->open(this);
        Q_EMIT opened();
    }
}

void content_widget::close()
{
    if (m_anchor)
    {
        m_anchor->close(this);
        Q_EMIT closed();
    }
}

void content_widget::closeEvent(QCloseEvent* event){Q_UNUSED(event)}

QString content_widget::name()
{
    return m_name;
}

QIcon content_widget::icon()
{
    return m_icon;
}

void content_widget::set_anchor(hal_content_anchor* anchor)
{
    m_anchor = anchor;
}

void content_widget::set_icon(QIcon icon)
{
    m_icon = icon;
}

void content_widget::setup_toolbar(toolbar* toolbar){Q_UNUSED(toolbar)}

QList<QShortcut*> content_widget::create_shortcuts()
{
    return QList<QShortcut*>();
}

void content_widget::repolish()
{
    QStyle* s = style();

    s->unpolish(this);
    s->polish(this);

    //    if (!m_icon_path.isEmpty())
    //        m_icon_label->setPixmap(gui_utility::get_styled_svg_icon(m_icon_style, m_icon_path).pixmap(QSize(20, 20)));
}

QString content_widget::icon_style()
{
    return m_icon_style;
}

QString content_widget::icon_path()
{
    return m_icon_path;
}

void content_widget::set_icon_style(const QString& style)
{
    if (m_icon_style == style)
        return;

    m_icon_style = style;
    repolish();
}

void content_widget::set_icon_path(const QString& path)
{
    if (m_icon_path == path)
        return;

    m_icon_path = path;
    repolish();
}
}
