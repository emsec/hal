#include "expanding_list/expanding_list_button.h"

#include "gui_utils/graphics.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QStyle>

namespace hal
{
    ExpandingListButton::ExpandingListButton(QWidget* parent)
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

    void ExpandingListButton::enterEvent(QEvent* event)
    {
        Q_UNUSED(event)

        m_hover = true;
        repolish();
    }

    void ExpandingListButton::leaveEvent(QEvent* event)
    {
        Q_UNUSED(event)

        m_hover = false;
        repolish();
    }

    void ExpandingListButton::mousePressEvent(QMouseEvent* event)
    {
        Q_UNUSED(event)

        Q_EMIT clicked();
        event->accept();
    }

    bool ExpandingListButton::hover()
    {
        return m_hover;
    }

    bool ExpandingListButton::selected()
    {
        return m_selected;
    }

    QString ExpandingListButton::type()
    {
        return m_type;
    }

    QString ExpandingListButton::icon_style()
    {
        return m_icon_style;
    }

    void ExpandingListButton::set_selected(bool selected)
    {
        if (m_selected == selected)
            return;

        m_selected = selected;
        repolish();
    }

    void ExpandingListButton::set_type(const QString& type)
    {
        if (m_type == type)
            return;

        m_type = type;
        repolish();
    }

    void ExpandingListButton::set_icon_style(const QString& style)
    {
        if (m_icon_style == style)
            return;

        m_icon_style = style;
        repolish();
    }

    void ExpandingListButton::set_icon_path(const QString& path)
    {
        if (m_icon_path == path)
            return;

        m_icon_path = path;
        repolish();
    }

    void ExpandingListButton::set_text(const QString& text)
    {
        m_text_label->setText(text);
    }

    void ExpandingListButton::repolish()
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
