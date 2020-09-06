#include "gui/welcome_screen/get_in_touch_item.h"

#include "gui/gui_utils/graphics.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QStyle>
#include <QVBoxLayout>

namespace hal
{
    GetInTouchItem::GetInTouchItem(const QString& title, const QString& description, QWidget* parent)
        : QFrame(parent), m_horizontal_layout(new QHBoxLayout()), m_icon_label(new QLabel()), m_vertical_layout(new QVBoxLayout()), m_title_label(new QLabel()), m_description_label(new QLabel()),
          m_animation(new QPropertyAnimation(this)), m_hover(false)
    {
        m_horizontal_layout->setContentsMargins(0, 0, 0, 0);
        m_horizontal_layout->setSpacing(0);

        m_icon_label->setObjectName("icon-label");
        m_icon_label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        m_vertical_layout->setContentsMargins(0, 0, 0, 0);
        m_vertical_layout->setSpacing(0);

        m_title_label->setObjectName("title-label");
        m_title_label->setText(title);

        m_description_label->setObjectName("description-label");
        m_description_label->setText(description);
        m_description_label->setWordWrap(true);

        setLayout(m_horizontal_layout);
        m_horizontal_layout->addWidget(m_icon_label);
        m_horizontal_layout->setAlignment(m_icon_label, Qt::AlignTop);
        m_horizontal_layout->addLayout(m_vertical_layout);
        m_vertical_layout->addWidget(m_title_label);
        m_vertical_layout->addWidget(m_description_label);

        ensurePolished();
        repolish();
    }

    void GetInTouchItem::enterEvent(QEvent* event)
    {
        Q_UNUSED(event)

        m_hover = true;
        repolish();
    }

    void GetInTouchItem::leaveEvent(QEvent* event)
    {
        Q_UNUSED(event)

        m_hover = false;
        repolish();
    }

    void GetInTouchItem::mousePressEvent(QMouseEvent* event)
    {
        Q_UNUSED(event)

        event->accept();
        Q_EMIT clicked();
    }

    void GetInTouchItem::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        s->unpolish(m_icon_label);
        s->polish(m_icon_label);

        s->unpolish(m_title_label);
        s->polish(m_title_label);

        s->unpolish(m_description_label);
        s->polish(m_description_label);

        if (!m_icon_path.isEmpty())
            m_icon_label->setPixmap(gui_utility::get_styled_svg_icon(m_icon_style, m_icon_path).pixmap(QSize(17, 17)));
    }

    bool GetInTouchItem::hover()
    {
        return m_hover;
    }

    QString GetInTouchItem::icon_path()
    {
        return m_icon_path;
    }

    QString GetInTouchItem::icon_style()
    {
        return m_icon_style;
    }

    void GetInTouchItem::set_hover_active(bool active)
    {
        m_hover = active;
    }

    void GetInTouchItem::set_icon_path(const QString& path)
    {
        m_icon_path = path;
    }

    void GetInTouchItem::set_icon_style(const QString& style)
    {
        m_icon_style = style;
    }
}
