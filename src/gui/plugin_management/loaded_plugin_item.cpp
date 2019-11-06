#include "plugin_management/loaded_plugin_item.h"

#include "gui_utils/graphics.h"

#include <QApplication>
#include <QDrag>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QStyle>
#include <QVBoxLayout>

bool loaded_plugin_item::s_drag_in_progress = false;
QPoint loaded_plugin_item::s_drag_start_position;

loaded_plugin_item::loaded_plugin_item(const QString& name, QWidget* parent)
    : QFrame(parent), m_horizontal_layout(new QHBoxLayout()), m_icon_label(new QLabel()), m_vertical_layout(new QVBoxLayout()), m_name_label(new QLabel()), m_description_label(new QLabel()),
      m_animation(new QPropertyAnimation()), m_hover(false)
{
    m_name = name;
    //    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_horizontal_layout->setContentsMargins(0, 0, 0, 0);
    m_horizontal_layout->setSpacing(0);

    //    m_icon_label->setObjectName("icon-label");
    //    m_icon_label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    //    m_horizontal_layout->addWidget(m_icon_label);
    //    m_horizontal_layout->setAlignment(m_icon_label, Qt::AlignTop);

    //    m_vertical_layout->setContentsMargins(0, 0, 0, 0);
    //    m_vertical_layout->setSpacing(0);

    m_name_label->setObjectName("name-label");
    m_name_label->setText(name);
    //    m_description_label->setObjectName("description-label");
    //    m_description_label->setText(description);
    //    m_description_label->setWordWrap(true);
    //    m_vertical_layout->addWidget(m_description_label);

    setLayout(m_horizontal_layout);
    m_horizontal_layout->addWidget(m_name_label);

    repolish();
}

void loaded_plugin_item::enterEvent(QEvent* event)
{
    if (event->type() == QEvent::Enter)
    {
        m_hover = true;
        repolish();
    }
}

void loaded_plugin_item::leaveEvent(QEvent* event)
{
    if (event->type() == QEvent::Leave)
    {
        if (s_drag_in_progress)
            exec_drag();

        m_hover = false;
        repolish();
    }
}

void loaded_plugin_item::mouseMoveEvent(QMouseEvent* event)
{
    if (!s_drag_in_progress)
        return;
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - s_drag_start_position).manhattanLength() < QApplication::startDragDistance())
        return;

    exec_drag();
}

void loaded_plugin_item::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        s_drag_in_progress    = true;
        s_drag_start_position = event->pos();
    }
    // SIGNAL CLICKED
    event->accept();
}

void loaded_plugin_item::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        s_drag_in_progress = false;

    // SIGNAL CLICKED
    event->accept();
}

void loaded_plugin_item::repolish()
{
    QStyle* s = style();

    s->unpolish(this);
    s->polish(this);

    s->unpolish(m_icon_label);
    s->polish(m_icon_label);

    s->unpolish(m_name_label);
    s->polish(m_name_label);

    s->unpolish(m_description_label);
    s->polish(m_description_label);

    if (!m_icon_path.isEmpty())
        m_icon_label->setPixmap(gui_utility::get_styled_svg_icon(m_icon_style, m_icon_path).pixmap(QSize(17, 17)));
}

void loaded_plugin_item::exec_drag()
{
    s_drag_in_progress   = false;
    QDrag* drag          = new QDrag(this);
    QMimeData* mime_data = new QMimeData();
    mime_data->setData("hal/plugin_name", m_name_label->text().toUtf8());
    mime_data->setData("hal/item_height", QString::number(height()).toUtf8());
    drag->setMimeData(mime_data);
    drag->setPixmap(grab());
    drag->setHotSpot(QPoint(drag->pixmap().width() / 2, drag->pixmap().height() / 2));

    m_hover = false;
    m_name_label->setText("");
    repolish();
    drag->exec();
    m_name_label->setText(m_name);
}

bool loaded_plugin_item::hover()
{
    return m_hover;
}

QString loaded_plugin_item::icon_path()
{
    return m_icon_path;
}

QString loaded_plugin_item::icon_style()
{
    return m_icon_style;
}

QString loaded_plugin_item::name()
{
    return m_name_label->text();
}

void loaded_plugin_item::set_hover_active(bool active)
{
    m_hover = active;
}

void loaded_plugin_item::set_icon_path(const QString& path)
{
    m_icon_path = path;
}

void loaded_plugin_item::set_icon_style(const QString& style)
{
    m_icon_style = style;
}
