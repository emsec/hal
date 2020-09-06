#include "gui/searchbar/searchbar.h"

#include "gui/gui_utils/graphics.h"
#include "gui/label_button/label_button.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QStyle>
#include <QToolButton>

namespace hal
{
    Searchbar::Searchbar(QWidget* parent)
        : QFrame(parent), m_layout(new QHBoxLayout()), m_search_icon_label(new QLabel()), m_LineEdit(new QLineEdit()), m_clear_icon_label(new QLabel()), m_mode_button(new QPushButton()),
          m_down_button(new QToolButton()), m_up_button(new QToolButton())
    {
        setLayout(m_layout);

        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);

        ensurePolished();

        m_search_icon_label->setPixmap(gui_utility::get_styled_svg_icon(m_search_icon_style, m_search_icon).pixmap(QSize(16, 16)));
        m_LineEdit->setPlaceholderText("Search");
        m_clear_icon_label->setPixmap(gui_utility::get_styled_svg_icon(m_clear_icon_style, m_clear_icon).pixmap(QSize(10, 10)));
        m_mode_button->setText("Mode");

        //Placeholder icons get better ones
        m_down_button->setIcon(QIcon(":/icons/arrow-down"));
        m_up_button->setIcon(QIcon(":/icons/arrow-up"));

        m_search_icon_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        m_LineEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        m_mode_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

        m_layout->addWidget(m_search_icon_label);
        m_layout->addWidget(m_LineEdit);
        //m_layout->addWidget(m_clear_icon_label);
        //m_layout.addWidget(m_button);

        //Hack solution, because QPushButton not working as expected, fix
        LabelButton* hack = new LabelButton("Mode", this);
        hack->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        m_layout->addWidget(hack);

        //m_layout.addWidget(&m_down_button);
        //m_layout.addWidget(&m_up_button);

        setFrameStyle(QFrame::NoFrame);

        connect(m_LineEdit, &QLineEdit::textEdited, this, &Searchbar::handle_text_edited);
        connect(m_LineEdit, &QLineEdit::returnPressed, this, &Searchbar::handle_return_pressed);
        connect(hack, &LabelButton::clicked, this, &Searchbar::handle_mode_clicked);

        setFocusProxy(m_LineEdit);
    }

    QString Searchbar::search_icon() const
    {
        return m_search_icon;
    }

    QString Searchbar::search_icon_style() const
    {
        return m_search_icon_style;
    }

    QString Searchbar::clear_icon() const
    {
        return m_clear_icon;
    }

    QString Searchbar::clear_icon_style() const
    {
        return m_clear_icon_style;
    }

    void Searchbar::set_search_icon(const QString& icon)
    {
        m_search_icon = icon;
    }

    void Searchbar::set_search_icon_style(const QString& style)
    {
        m_search_icon_style = style;
    }

    void Searchbar::set_clear_icon(const QString& icon)
    {
        m_clear_icon = icon;
    }

    void Searchbar::set_clear_icon_style(const QString& style)
    {
        m_clear_icon_style = style;
    }

    void Searchbar::set_placeholder_text(const QString& text)
    {
        m_LineEdit->setPlaceholderText(text);
    }

    void Searchbar::set_mode_button_text(const QString& text)
    {
        m_mode_button->setText(text);
    }

    void Searchbar::clear()
    {
        m_LineEdit->clear();
        repolish();

        Q_EMIT text_edited(m_LineEdit->text());
    }

    QString Searchbar::get_current_text()
    {
        return m_LineEdit->text();
    }

    void Searchbar::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        s->unpolish(m_search_icon_label);
        s->polish(m_search_icon_label);

        s->unpolish(m_LineEdit);
        s->polish(m_LineEdit);

        s->unpolish(m_mode_button);
        s->polish(m_mode_button);
    }

    void Searchbar::handle_text_edited(const QString& text)
    {
        repolish();
        Q_EMIT text_edited(text);
    }

    void Searchbar::handle_return_pressed()
    {
        Q_EMIT return_pressed();
    }

    void Searchbar::handle_mode_clicked()
    {
        Q_EMIT mode_clicked();
    }
}
