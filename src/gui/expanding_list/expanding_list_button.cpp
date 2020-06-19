#include "expanding_list/expanding_list_button.h"

#include "gui_utils/graphics.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QStyle>

namespace hal
{
    expanding_list_button::expanding_list_button(QWidget* parent)
        : QFrame(parent), m_layout(new QHBoxLayout()), m_left_border(new QFrame()), m_icon_label(new QLabel()), m_text_label(new QLabel()), m_right_border(new QFrame()), m_hover(false), m_selected(false),
          m_type(""), m_icon_style(""), m_icon_path("")
    {
        setLayout(m_layout);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);

        m_left_border->setObjectName("left-border");
        m_left_border->setFrameStyle(QFrame::NoFrame);
        m_layout->addWidget(m_left_border);

        m_icon_label->setObjectName("icon-label");
        m_icon_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        m_layout->addWidget(m_icon_label);

        m_text_label->setObjectName("text-label");
        m_layout->addWidget(m_text_label);

        m_right_border->setObjectName("right-border");
        m_right_border->setFrameStyle(QFrame::NoFrame);
        m_layout->addWidget(m_right_border);
    }

    void expanding_list_button::enterEvent(QEvent* event)
    {
        Q_UNUSED(event)

        m_hover = true;
        repolish();
    }

    void expanding_list_button::leaveEvent(QEvent* event)
    {
        Q_UNUSED(event)

        m_hover = false;
        repolish();
    }

    void expanding_list_button::mousePressEvent(QMouseEvent* event)
    {
        Q_UNUSED(event)

        Q_EMIT clicked();
        event->accept();
    }

    bool expanding_list_button::hover()
    {
        return m_hover;
    }

    bool expanding_list_button::selected()
    {
        return m_selected;
    }

    QString expanding_list_button::type()
    {
        return m_type;
    }

    QString expanding_list_button::icon_style()
    {
        return m_icon_style;
    }

    void expanding_list_button::set_selected(bool selected)
    {
        if (m_selected == selected)
            return;

        m_selected = selected;
        repolish();
    }

    void expanding_list_button::set_type(const QString& type)
    {
        if (m_type == type)
            return;

        m_type = type;
        repolish();
    }

    void expanding_list_button::set_icon_style(const QString& style)
    {
        if (m_icon_style == style)
            return;

        m_icon_style = style;
        repolish();
    }

    void expanding_list_button::set_icon_path(const QString& path)
    {
        if (m_icon_path == path)
            return;

        m_icon_path = path;
        repolish();
    }

    void expanding_list_button::set_text(const QString& text)
    {
        m_text_label->setText(text);
    }

    void expanding_list_button::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        s->unpolish(m_left_border);
        s->polish(m_left_border);

        s->unpolish(m_icon_label);
        s->polish(m_icon_label);

        s->unpolish(m_text_label);
        s->polish(m_text_label);

        s->unpolish(m_right_border);
        s->polish(m_right_border);

        if (!m_icon_path.isEmpty())
            m_icon_label->setPixmap(gui_utility::get_styled_svg_icon(m_icon_style, m_icon_path).pixmap(QSize(20, 20)));
    }
}
