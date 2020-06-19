#include "docking_system/dock_button.h"
#include "content_widget/content_widget.h"
#include "core/log.h"
#include "gui_globals.h"
#include "gui_utils/graphics.h"
#include <QPainter>

namespace hal
{
    dock_button::dock_button(content_widget* widget, button_orientation orientation, QObject* eventFilter, QWidget* parent) : QToolButton(parent), m_widget(widget), m_orientation(orientation)
    {
        setText(m_widget->name());
        setCheckable(true);
        setStyleSheet("QToolButton { font-family                : \"Iosevka\";padding: 0; margin: 0; }");
        setIcon(m_widget->icon());
        installEventFilter(eventFilter);

        m_icon_size       = 14;
        m_width_padding   = 16;
        m_height_padding  = 0;
        m_relative_height = 18;
        adjust_size();

        connect(this, &dock_button::clicked, this, &dock_button::handle_clicked);
    }

    void dock_button::paintEvent(QPaintEvent* event)
    {
        Q_UNUSED(event)
        auto font = property("font").value<QFont>();
        QPainter painter(this);
        painter.setFont(font);
        painter.setPen(Qt::white);
        if (underMouse())
        {
            painter.fillRect(0, 0, m_width, m_height, QColor("#666769"));
        }
        else if (isChecked())
        {
            painter.fillRect(0, 0, m_width, m_height, QColor("#808080"));
        }

        switch (m_orientation)
        {
            case button_orientation::horizontal:
                painter.translate(0, -1);
                break;
            case button_orientation::vertical_up:
                painter.translate(-1, m_height);
                painter.rotate(270);
                break;
            case button_orientation::vertical_down:
                painter.translate(m_width + 1, 0);
                painter.rotate(90);
                break;
        }
        icon().paint(&painter, 0, (m_relative_height / 2) - (m_icon_size / 2), m_icon_size, (m_relative_height / 2) - (m_icon_size / 2) + m_icon_size);
        painter.drawText(QRectF(QRect(m_icon_size, 0, m_relative_width, m_relative_height)), Qt::AlignVCenter, text());
    }

    void dock_button::adjust_size()
    {
        auto font = property("font").value<QFont>();
        //, QFont::PreferAntialias);
        QFontMetrics fm(font);
        int textwidth    = fm.width(text());
        m_relative_width = m_icon_size + textwidth + m_width_padding;

        if (m_orientation == button_orientation::horizontal)
        {
            m_width  = m_relative_width;
            m_height = m_relative_height;
        }
        else
        {
            m_width  = m_relative_height;
            m_height = m_relative_width;
        }

        setFixedHeight(m_height);
        setFixedWidth(m_width);
    }

    int dock_button::relative_width()
    {
        return m_relative_width;
    }

    void dock_button::handle_clicked(bool checked)
    {
        if (checked)
            m_widget->open();
        else
            m_widget->close();
    }

    content_widget* dock_button::widget()
    {
        return m_widget;
    }

    void dock_button::hide()
    {
        QWidget::hide();
        m_hidden = true;
    }

    void dock_button::show()
    {
        QWidget::show();
        m_hidden = false;
    }

    bool dock_button::hidden()
    {
        return m_hidden;
    }

    bool dock_button::available()
    {
        return m_available;
    }

    void dock_button::set_available(bool available)
    {
        m_available = available;
    }

    void dock_button::set_relative_height(int height)
    {
        m_relative_height = height;
        adjust_size();
    }
}
