#include "docking_system/dock_bar.h"
#include "content_frame/content_frame.h"
#include "content_widget/content_widget.h"
#include "docking_system/content_drag_relay.h"
#include "docking_system/dock_mime_data.h"
#include "gui_globals.h"
#include "hal_content_anchor/hal_content_anchor.h"
#include <QApplication>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QListIterator>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>

QPoint dock_bar::s_drag_start_position;
dock_button* dock_bar::s_drag_button = nullptr;
int dock_bar::s_begin_drop_range     = 0;
int dock_bar::s_end_drop_range       = 0;
int dock_bar::s_drop_spacing         = 0;
dock_button* dock_bar::s_move_marker = nullptr;

dock_bar::dock_bar(Qt::Orientation orientation, button_orientation b_orientation, QWidget* parent) : QFrame(parent), m_button_orientation(b_orientation)
{
    Q_UNUSED(orientation)

    setAcceptDrops(true);

    //USE CONTENTSRECT / FRAMEWIDTH TO CALCULATE CORRECT BUTTON POSITIONS
}

void dock_bar::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if (!s_drag_button)
        return;
    if ((event->pos() - s_drag_start_position).manhattanLength() < QApplication::startDragDistance())
        return;

    QDrag* drag              = new QDrag(this);
    dock_mime_data* mimeData = new dock_mime_data(s_drag_button->widget());
    drag->setPixmap(s_drag_button->grab());
    drag->setHotSpot(QPoint(drag->pixmap().width() / 2, drag->pixmap().height() / 2));
    s_drag_button->hide();
    s_drag_button->setChecked(false);
    s_drag_button->widget()->close();
    s_drag_button->set_available(false);
    s_drop_spacing = s_drag_button->relative_width();
    drag->setMimeData(mimeData);
    collapse_buttons();
    content_drag_relay::instance()->relay_drag_start();
    Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
    if (!(dropAction & Qt::MoveAction))
    {
        Q_EMIT stop_animations();
        s_drag_button->set_available(true);
        rearrange_buttons();
        s_drag_button->show();
    }
    content_drag_relay::instance()->relay_drag_end();
    s_drag_button = nullptr;
}

void dock_bar::dragEnterEvent(QDragEnterEvent* event)
{
    const dock_mime_data* mime_data = qobject_cast<const dock_mime_data*>(event->mimeData());
    if (mime_data)
    {
        event->acceptProposedAction();
        s_begin_drop_range = 0;
        s_end_drop_range   = 0;
        s_move_marker      = nullptr;
    }
}

void dock_bar::dragMoveEvent(QDragMoveEvent* event)
{
    int cursor_position;
    if (m_button_orientation == button_orientation::horizontal)
        cursor_position = event->pos().x();
    else
        cursor_position = event->pos().y();

    if (cursor_position < s_begin_drop_range || cursor_position > s_end_drop_range)
    {
        int button_position = m_button_offset;
        QList<dock_button*>::iterator button_iterator;
        QList<QPropertyAnimation*> animations;
        bool already_executed    = false;
        bool move_marker_reached = false;

        for (button_iterator = m_buttons.begin(); button_iterator != m_buttons.end(); ++button_iterator)
        {
            if ((*button_iterator)->available())
            {
                if (button_position + (*button_iterator)->relative_width() < cursor_position)
                {
                    if (*button_iterator == s_move_marker)
                        move_marker_reached = true;    //executed once at max

                    if (move_marker_reached)
                    {
                        QPropertyAnimation* animation = new QPropertyAnimation(*button_iterator, "pos");
                        animation->setDuration(200);
                        animation->setStartValue((*button_iterator)->pos());
                        if (m_button_orientation == button_orientation::horizontal)
                        {
                            animation->setEndValue(QPoint(button_position, 0));
                        }
                        else
                        {
                            animation->setEndValue(QPoint(0, button_position));
                        }
                        animation->start();
                    }
                }
                else
                {
                    if (!already_executed)
                    {
                        s_begin_drop_range = button_position;
                        s_end_drop_range   = button_position + (*button_iterator)->relative_width();
                        s_move_marker      = *button_iterator;
                        already_executed   = true;
                    }

                    QPropertyAnimation* animation = new QPropertyAnimation(*button_iterator, "pos");
                    animation->setDuration(200);
                    animation->setStartValue((*button_iterator)->pos());

                    if (m_button_orientation == button_orientation::horizontal)
                    {
                        animation->setEndValue(QPoint(button_position + s_drop_spacing, 0));
                    }
                    else
                    {
                        animation->setEndValue(QPoint(0, button_position + s_drop_spacing));
                    }
                    animations.append(animation);
                }
                button_position += (*button_iterator)->relative_width() + m_button_spacing;
            }
        }
        for (QPropertyAnimation* animation : animations)
        {
            animation->start();
        }
        if (already_executed)
            return;
        s_begin_drop_range = 0;
        s_end_drop_range   = 0;
        s_move_marker      = nullptr;
    }
}

void dock_bar::dragLeaveEvent(QDragLeaveEvent* event)
{
    Q_UNUSED(event)
    collapse_buttons();
}

void dock_bar::dropEvent(QDropEvent* event)
{
    const dock_mime_data* mimedata = qobject_cast<const dock_mime_data*>(event->mimeData());
    if (mimedata)
    {
        event->acceptProposedAction();
        content_widget* widget = mimedata->widget();
        widget->remove();

        if (s_move_marker == nullptr)
        {
            m_anchor->add(widget, count());
            return;
        }

        int index = 0;
        for (dock_button* button : m_buttons)
        {
            if (button == s_move_marker)
            {
                m_anchor->add(widget, index);
                return;
            }
            index++;
        }
    }
}

bool dock_bar::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->buttons() & Qt::LeftButton)
        {
            s_drag_start_position = mapFromGlobal(mouseEvent->globalPos());
            s_drag_button         = static_cast<dock_button*>(watched);
        }
    }
    return false;
}

QSize dock_bar::sizeHint() const
{
    int width = 0;
    for (dock_button* button : m_buttons)
        width += button->width();

    width += m_buttons.size() * m_button_spacing;    //+ m_button_offset

    return QSize(width, height());
}

QSize dock_bar::minimumSizeHint() const
{
    int width = 0;
    for (dock_button* button : m_buttons)
        width += button->width();

    width += m_buttons.size() * m_button_spacing;    //+ m_button_offset

    return QSize(width, height());
}

void dock_bar::set_anchor(hal_content_anchor* anchor)
{
    m_anchor = anchor;
}

void dock_bar::set_autohide(bool autohide)
{
    m_autohide = autohide;
}

bool dock_bar::unused()
{
    bool no_button_visible = true;
    for (dock_button* button : m_buttons)
    {
        if (!button->hidden())
        {
            no_button_visible = false;
            break;
        }
    }
    if (no_button_visible)
        return true;
    return false;
}

int dock_bar::count()
{
    return m_buttons.length();
}

int dock_bar::index(content_widget* widget)
{
    int index = 0;
    for (dock_button* button : m_buttons)
    {
        if (button->widget() == widget)
            return index;
        index++;
    }
    return -1;
}

void dock_bar::rearrange_buttons()
{
    int position = m_button_offset;
    for (dock_button* button : m_buttons)
    {
        if (button->available())
        {
            if (m_button_orientation == button_orientation::horizontal)
            {
                button->move(position, 0);
                position += button->relative_width() + m_button_spacing;
            }
            else
            {
                button->move(0, position);
                position += button->relative_width() + m_button_spacing;
            }
        }
    }
}

void dock_bar::collapse_buttons()
{
    int position = m_button_offset;
    for (dock_button* button : m_buttons)
    {
        if (button->available())
        {
            QPropertyAnimation* animation = new QPropertyAnimation(button, "pos");
            animation->setDuration(200);
            animation->setStartValue(button->pos());

            if (m_button_orientation == button_orientation::horizontal)
            {
                animation->setEndValue(QPoint(position, 0));
            }
            else
            {
                animation->setEndValue(QPoint(0, position));
            }
            position += button->relative_width() + m_button_spacing;
            connect(this, SIGNAL(stop_animations()), animation, SLOT(stop()));
            animation->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }
}

void dock_bar::add_button(content_widget* widget, int index)
{
    dock_button* button = new dock_button(widget, m_button_orientation, this, nullptr);
    if (m_button_orientation == button_orientation::horizontal)
        button->set_relative_height(height());
    else
        button->set_relative_height(width());

    m_buttons.insert(index, button);
    rearrange_buttons();
    button->setParent(this);
    button->show();
    show();
    updateGeometry();
}

bool dock_bar::remove_button(content_widget* widget)
{
    for (dock_button* button : m_buttons)
    {
        if (button->widget() == widget)
        {
            m_buttons.removeOne(button);
            button->hide();
            rearrange_buttons();
            delete button;

            if (m_autohide && unused())
                hide();
            return true;
        }
    }
    return false;
}

bool dock_bar::remove_button(int index)
{
    if (0 <= index && index < m_buttons.size())
    {
        dock_button* button = m_buttons.at(index);
        button->hide();
        m_buttons.removeAt(index);
        button->close();
        rearrange_buttons();
        if (m_autohide && unused())
            hide();
        return true;
    }
    return false;
}

void dock_bar::detach_button(content_widget* widget)
{
    for (dock_button* button : m_buttons)
    {
        if (button->widget() == widget)
        {
            button->hide();
            button->set_available(false);
            button->setChecked(false);
            rearrange_buttons();

            if (m_autohide && unused())
                hide();
            break;
        }
    }
}

void dock_bar::reattach_button(content_widget* widget)
{
    for (dock_button* button : m_buttons)
    {
        if (button->widget() == widget)
        {
            button->set_available(true);
            rearrange_buttons();
            button->show();
            show();
            break;
        }
    }
}

void dock_bar::check_button(content_widget* widget)
{
    for (dock_button* button : m_buttons)
    {
        if (button->widget() == widget)
        {
            button->setChecked(true);
            break;
        }
    }
}

void dock_bar::uncheck_button(content_widget* widget)
{
    for (dock_button* button : m_buttons)
    {
        if (button->widget() == widget)
        {
            button->setChecked(false);
            break;
        }
    }
}

content_widget* dock_bar::widget_at(int index)
{
    if (0 <= index && index < m_buttons.size())
        return m_buttons.at(index)->widget();
    return nullptr;
}

content_widget* dock_bar::next_available_widget(int index)
{
    if (index < 0)
    {
        for (dock_button* button : m_buttons)
        {
            if (!button->hidden())
                return button->widget();
        }
        return nullptr;
    }
    if (index >= m_buttons.size())
    {
        content_widget* widget = nullptr;
        for (dock_button* button : m_buttons)
        {
            if (!button->hidden())
                widget = button->widget();
        }
        return widget;
    }
    int current_index      = 0;
    content_widget* widget = nullptr;
    for (dock_button* button : m_buttons)
    {
        if (!button->hidden())
        {
            widget = button->widget();
            if (current_index >= index)
                return widget;
        }
        current_index++;
    }
    return widget;
}

void dock_bar::handle_drag_start()
{
    show();
}

void dock_bar::handle_drag_end()
{
    if (m_autohide && unused())
        hide();
}
