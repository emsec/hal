#include "content_widget/content_widget.h"

#include "gui_utils/graphics.h"
#include "content_anchor/content_anchor.h"
#include "toolbar/toolbar.h"

#include <QShortcut>
#include <QStyle>
#include <QVBoxLayout>

namespace hal
{
    ContentWidget::ContentWidget(QString name, QWidget* parent) : Widget(parent), m_name(name), m_content_layout(new QVBoxLayout())
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

    void ContentWidget::remove()
    {
        if (m_anchor)
        {
            m_anchor->remove(this);
            Q_EMIT removed();
        }
    }

    void ContentWidget::detach()
    {
        if (m_anchor)
        {
            m_anchor->detach(this);
            Q_EMIT detached();
        }
    }

    void ContentWidget::reattach()
    {
        if (m_anchor)
        {
            m_anchor->reattach(this);
            Q_EMIT reattached();
        }
    }

    void ContentWidget::open()
    {
        if (m_anchor)
        {
            m_anchor->open(this);
            Q_EMIT opened();
        }
    }

    void ContentWidget::close()
    {
        if (m_anchor)
        {
            m_anchor->close(this);
            Q_EMIT closed();
        }
    }

    void ContentWidget::closeEvent(QCloseEvent* event){Q_UNUSED(event)}

    QString ContentWidget::name()
    {
        return m_name;
    }

    QIcon ContentWidget::icon()
    {
        return m_icon;
    }

    void ContentWidget::set_anchor(ContentAnchor* anchor)
    {
        m_anchor = anchor;
    }

    void ContentWidget::set_icon(QIcon icon)
    {
        m_icon = icon;
    }

    void ContentWidget::setup_toolbar(Toolbar* Toolbar){Q_UNUSED(Toolbar)}

    QList<QShortcut*> ContentWidget::create_shortcuts()
    {
        return QList<QShortcut*>();
    }

    void ContentWidget::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        //    if (!m_icon_path.isEmpty())
        //        m_icon_label->setPixmap(gui_utility::get_styled_svg_icon(m_icon_style, m_icon_path).pixmap(QSize(20, 20)));
    }

    QString ContentWidget::icon_style()
    {
        return m_icon_style;
    }

    QString ContentWidget::icon_path()
    {
        return m_icon_path;
    }

    void ContentWidget::set_icon_style(const QString& style)
    {
        if (m_icon_style == style)
            return;

        m_icon_style = style;
        repolish();
    }

    void ContentWidget::set_icon_path(const QString& path)
    {
        if (m_icon_path == path)
            return;

        m_icon_path = path;
        repolish();
    }
}
