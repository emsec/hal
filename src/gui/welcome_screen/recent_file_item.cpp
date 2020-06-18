#include "welcome_screen/recent_file_item.h"

#include "netlist/event_system/event_controls.h"

#include "file_manager/file_manager.h"
#include "gui_utils/graphics.h"

#include <QEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QStyle>
#include <QVBoxLayout>
#include <QToolButton>
namespace hal{
recent_file_item::recent_file_item(const QString& file, QWidget* parent)
    : QFrame(parent), m_widget(new QWidget()), m_horizontal_layout(new QHBoxLayout()), m_icon_label(new QLabel()), m_vertical_layout(new QVBoxLayout()), m_name_label(new QLabel()),
      m_path_label(new QLabel()), m_animation(new QPropertyAnimation()), m_remove_button(new QToolButton(this)), m_hover(false), m_disabled(false)
{
    m_horizontal_layout->setContentsMargins(0, 0, 0, 0);
    m_horizontal_layout->setSpacing(0);

    m_icon_label->setObjectName("icon-label");
    m_icon_label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    m_vertical_layout->setContentsMargins(0, 0, 0, 0);
    m_vertical_layout->setSpacing(0);

    m_name_label->setObjectName("name-label");
    m_path_label->setObjectName("path-label");


    m_remove_button->setText("x");
    connect(m_remove_button, &QToolButton::clicked, this, &recent_file_item::handle_close_requested);

    setLayout(m_horizontal_layout);
    m_horizontal_layout->addWidget(m_icon_label);
    m_horizontal_layout->setAlignment(m_icon_label, Qt::AlignTop);
    m_horizontal_layout->addLayout(m_vertical_layout);
    m_horizontal_layout->addWidget(m_remove_button);
    m_vertical_layout->addWidget(m_name_label);
    m_vertical_layout->addWidget(m_path_label);

    m_file = file;
    QFileInfo info(file);
    m_name_label->setText(info.fileName());
    //m_path = info.canonicalPath();
    m_path = info.absolutePath();

    m_path_label->ensurePolished();
    int width                 = m_path_label->width();
    QFontMetrics font_metrics = m_path_label->fontMetrics();
    m_path_label->setText(font_metrics.elidedText(m_path, Qt::TextElideMode::ElideLeft, width));

    repolish();
}

void recent_file_item::enterEvent(QEvent* event)
{
    Q_UNUSED(event)
    if(m_disabled)
        return;

    m_hover = true;
    repolish();
}

void recent_file_item::leaveEvent(QEvent* event)
{
    Q_UNUSED(event)
    if(m_disabled)
        return;

    m_hover = false;
    repolish();
}

void recent_file_item::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event)

    if(m_disabled)
        return;

    if(event->button() == Qt::MouseButton::LeftButton)
    {
        // DEBUG -- REMOVE WHEN GUI CAN HANDLE EVENTS DURING CREATION
        event_controls::enable_all(false);
        file_manager::get_instance()->open_file(m_file);
        // DEBUG -- REMOVE WHEN GUI CAN HANDLE EVENTS DURING CREATION
        event_controls::enable_all(true);
        event->accept();
    }
}

QSize recent_file_item::sizeHint() const
{
    return m_widget->sizeHint();
}

bool recent_file_item::eventFilter(QObject* object, QEvent* event)
{
    if (object == m_path_label && event->type() == QEvent::Resize)
    {
        QResizeEvent* resize_event = static_cast<QResizeEvent*>(event);
        int width                  = resize_event->size().width();

        m_path_label->ensurePolished();
        QFontMetrics font_metrics = m_path_label->fontMetrics();
        m_path_label->setText(font_metrics.elidedText(m_path, Qt::TextElideMode::ElideLeft, width));
    }

    return false;
}

QString recent_file_item::file()
{
    return m_file;
}

void recent_file_item::repolish()
{
    QStyle* s = style();

    s->unpolish(this);
    s->polish(this);

    s->unpolish(m_icon_label);
    s->polish(m_icon_label);

    s->unpolish(m_name_label);
    s->polish(m_name_label);

    s->unpolish(m_path_label);
    s->polish(m_path_label);

    if (!m_icon_path.isEmpty())
        m_icon_label->setPixmap(gui_utility::get_styled_svg_icon(m_icon_style, m_icon_path).pixmap(QSize(17, 17)));
}

bool recent_file_item::hover()
{
    return m_hover;
}

bool recent_file_item::disabled()
{
    return m_disabled;
}

QString recent_file_item::icon_path()
{
    return m_icon_path;
}

QString recent_file_item::icon_style()
{
    return m_icon_style;
}

void recent_file_item::set_hover_active(bool active)
{
    m_hover = active;
}

void recent_file_item::set_disabled(bool disable)
{
    m_disabled = disable;
    m_name_label->setText(m_name_label->text().append(" [Missing]"));
    m_hover = false;
}

void recent_file_item::set_icon_path(const QString& path)
{
    m_icon_path = path;
}

void recent_file_item::set_icon_style(const QString& style)
{
    m_icon_style = style;
}

void recent_file_item::handle_close_requested()
{
    Q_EMIT remove_requested(this);
}
}
