#include "gui/docking_system/dock_bar.h"
#include "gui/content_frame/content_frame.h"
#include "gui/content_widget/content_widget.h"
#include "gui/docking_system/content_drag_relay.h"
#include "gui/docking_system/dock_mime_data.h"
#include "gui/gui_globals.h"
#include "gui/content_anchor/content_anchor.h"
#include <QApplication>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QListIterator>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>

namespace hal
{
    QPoint DockBar::sDragStartPosition;
    DockButton* DockBar::sDragButton = nullptr;
    int DockBar::sBeginDropRange     = 0;
    int DockBar::sEndDropRange       = 0;
    int DockBar::sDropSpacing         = 0;
    DockButton* DockBar::sMoveMarker = nullptr;

    DockBar::DockBar(Qt::Orientation orientation, button_orientation b_orientation, QWidget* parent) : QFrame(parent), mButtonOrientation(b_orientation)
    {
        mOrientation = orientation;

        setAcceptDrops(true);

        //USE CONTENTSRECT / FRAMEWIDTH TO CALCULATE CORRECT BUTTON POSITIONS
    }

    void DockBar::mouseMoveEvent(QMouseEvent* event)
    {
        if (!(event->buttons() & Qt::LeftButton))
            return;
        if (!sDragButton)
            return;
        if ((event->pos() - sDragStartPosition).manhattanLength() < QApplication::startDragDistance())
            return;

        QDrag* drag              = new QDrag(this);
        DockMimeData* mimeData = new DockMimeData(sDragButton->widget());
        drag->setPixmap(sDragButton->grab());
        drag->setHotSpot(QPoint(drag->pixmap().width() / 2, drag->pixmap().height() / 2));
        sDragButton->hide();
        sDragButton->setChecked(false);
        sDragButton->widget()->close();
        sDragButton->setAvailable(false);
        sDropSpacing = sDragButton->relativeWidth();
        drag->setMimeData(mimeData);
        collapseButtons();
        ContentDragRelay::instance()->relayDragStart();
        Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
        if (!(dropAction & Qt::MoveAction))
        {
            Q_EMIT stopAnimations();
            sDragButton->setAvailable(true);
            rearrangeButtons();
            sDragButton->show();
        }
        ContentDragRelay::instance()->relayDragEnd();
        sDragButton = nullptr;
    }

    void DockBar::dragEnterEvent(QDragEnterEvent* event)
    {
        const DockMimeData* mime_data = qobject_cast<const DockMimeData*>(event->mimeData());
        if (mime_data)
        {
            event->acceptProposedAction();
            sBeginDropRange = 0;
            sEndDropRange   = 0;
            sMoveMarker      = nullptr;
        }
    }

    void DockBar::dragMoveEvent(QDragMoveEvent* event)
    {
        int cursor_position;
        if (mButtonOrientation == button_orientation::horizontal)
            cursor_position = event->pos().x();
        else
            cursor_position = event->pos().y();

        if (cursor_position < sBeginDropRange || cursor_position > sEndDropRange)
        {
            int button_position = mButtonOffset;
            QList<DockButton*>::iterator button_iterator;
            QList<QPropertyAnimation*> animations;
            bool already_executed    = false;
            bool move_marker_reached = false;

            for (button_iterator = mButtons.begin(); button_iterator != mButtons.end(); ++button_iterator)
            {
                if ((*button_iterator)->available())
                {
                    if (button_position + (*button_iterator)->relativeWidth() < cursor_position)
                    {
                        if (*button_iterator == sMoveMarker)
                            move_marker_reached = true;    //executed once at max

                        if (move_marker_reached)
                        {
                            QPropertyAnimation* animation = new QPropertyAnimation(*button_iterator, "pos");
                            animation->setDuration(200);
                            animation->setStartValue((*button_iterator)->pos());
                            if (mButtonOrientation == button_orientation::horizontal)
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
                            sBeginDropRange = button_position;
                            sEndDropRange   = button_position + (*button_iterator)->relativeWidth();
                            sMoveMarker      = *button_iterator;
                            already_executed   = true;
                        }

                        QPropertyAnimation* animation = new QPropertyAnimation(*button_iterator, "pos");
                        animation->setDuration(200);
                        animation->setStartValue((*button_iterator)->pos());

                        if (mButtonOrientation == button_orientation::horizontal)
                        {
                            animation->setEndValue(QPoint(button_position + sDropSpacing, 0));
                        }
                        else
                        {
                            animation->setEndValue(QPoint(0, button_position + sDropSpacing));
                        }
                        animations.append(animation);
                    }
                    button_position += (*button_iterator)->relativeWidth() + mButtonSpacing;
                }
            }
            for (QPropertyAnimation* animation : animations)
            {
                animation->start();
            }
            if (already_executed)
                return;
            sBeginDropRange = 0;
            sEndDropRange   = 0;
            sMoveMarker      = nullptr;
        }
    }

    void DockBar::dragLeaveEvent(QDragLeaveEvent* event)
    {
        Q_UNUSED(event)
        collapseButtons();
    }

    void DockBar::dropEvent(QDropEvent* event)
    {
        const DockMimeData* mimedata = qobject_cast<const DockMimeData*>(event->mimeData());
        if (mimedata)
        {
            event->acceptProposedAction();
            ContentWidget* widget = mimedata->widget();
            widget->remove();

            if (sMoveMarker == nullptr)
            {
                mAnchor->add(widget, count());
                return;
            }

            int index = 0;
            for (DockButton* button : mButtons)
            {
                if (button == sMoveMarker)
                {
                    mAnchor->add(widget, index);
                    return;
                }
                index++;
            }
        }
    }

    bool DockBar::eventFilter(QObject* watched, QEvent* event)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->buttons() & Qt::LeftButton)
            {
                sDragStartPosition = mapFromGlobal(mouseEvent->globalPos());
                sDragButton         = static_cast<DockButton*>(watched);
            }
        }
        return false;
    }

    QSize DockBar::sizeHint() const
    {
        if(mOrientation == Qt::Horizontal)
        {
            int width = 0;
            for (DockButton* button : mButtons)
                width += button->width();

            width += mButtons.size() * mButtonSpacing;    //+ mButtonOffset
            return QSize(width, height());
        }
        else
        {
            int height = 0;
            for (DockButton* button : mButtons)
                height += button->height();

            height += mButtons.size() * mButtonSpacing;    //+ mButtonOffset
            return QSize(width(), height);

        }
    }

    QSize DockBar::minimumSizeHint() const
    {
        if(mOrientation == Qt::Horizontal)
        {
            int width = 0;
            for (DockButton* button : mButtons)
                width += button->width();

            width += mButtons.size() * mButtonSpacing;    //+ mButtonOffset
            return QSize(width, height());
        }
        else
        {
            int height = 0;
            for (DockButton* button : mButtons)
                height += button->height();

            height += mButtons.size() * mButtonSpacing;    //+ mButtonOffset
            return QSize(width(), height);
        }
    }

    void DockBar::setAnchor(ContentAnchor* anchor)
    {
        mAnchor = anchor;
    }

    void DockBar::setAutohide(bool autohide)
    {
        mAutohide = autohide;
    }

    bool DockBar::unused()
    {
        bool no_button_visible = true;
        for (DockButton* button : mButtons)
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

    int DockBar::count()
    {
        return mButtons.length();
    }

    int DockBar::index(ContentWidget* widget)
    {
        int index = 0;
        for (DockButton* button : mButtons)
        {
            if (button->widget() == widget)
                return index;
            index++;
        }
        return -1;
    }

    void DockBar::rearrangeButtons()
    {
        int position = mButtonOffset;
        for (DockButton* button : mButtons)
        {
            if (button->available())
            {
                if (mButtonOrientation == button_orientation::horizontal)
                {
                    button->move(position, 0);
                    position += button->relativeWidth() + mButtonSpacing;
                }
                else
                {
                    button->move(0, position);
                    position += button->relativeWidth() + mButtonSpacing;
                }
            }
        }
    }

    void DockBar::collapseButtons()
    {
        int position = mButtonOffset;
        for (DockButton* button : mButtons)
        {
            if (button->available())
            {
                QPropertyAnimation* animation = new QPropertyAnimation(button, "pos");
                animation->setDuration(200);
                animation->setStartValue(button->pos());

                if (mButtonOrientation == button_orientation::horizontal)
                {
                    animation->setEndValue(QPoint(position, 0));
                }
                else
                {
                    animation->setEndValue(QPoint(0, position));
                }
                position += button->relativeWidth() + mButtonSpacing;
                connect(this, SIGNAL(stopAnimations()), animation, SLOT(stop()));
                animation->start(QAbstractAnimation::DeleteWhenStopped);
            }
        }
    }

    void DockBar::addButton(ContentWidget* widget, int index)
    {
        DockButton* button = new DockButton(widget, mButtonOrientation, this, nullptr);
        if (mButtonOrientation == button_orientation::horizontal)
            button->setRelativeHeight(height());
        else
            button->setRelativeHeight(width());

        mButtons.insert(index, button);
        rearrangeButtons();
        button->setParent(this);
        button->show();
        show();
        updateGeometry();
    }

    bool DockBar::removeButton(ContentWidget* widget)
    {
        for (DockButton* button : mButtons)
        {
            if (button->widget() == widget)
            {
                mButtons.removeOne(button);
                button->hide();
                rearrangeButtons();
                delete button;

                if (mAutohide && unused())
                    hide();
                updateGeometry();
                return true;
            }
        }
        return false;
    }

    bool DockBar::removeButton(int index)
    {
        if (0 <= index && index < mButtons.size())
        {
            DockButton* button = mButtons.at(index);
            button->hide();
            mButtons.removeAt(index);
            button->close();
            rearrangeButtons();
            if (mAutohide && unused())
                hide();
            updateGeometry();
            return true;
        }
        return false;
    }

    void DockBar::detachButton(ContentWidget* widget)
    {
        for (DockButton* button : mButtons)
        {
            if (button->widget() == widget)
            {
                button->hide();
                button->setAvailable(false);
                button->setChecked(false);
                rearrangeButtons();

                if (mAutohide && unused())
                    hide();
                break;
            }
        }
    }

    void DockBar::reattachButton(ContentWidget* widget)
    {
        for (DockButton* button : mButtons)
        {
            if (button->widget() == widget)
            {
                button->setAvailable(true);
                rearrangeButtons();
                button->show();
                show();
                break;
            }
        }
    }

    void DockBar::checkButton(ContentWidget* widget)
    {
        for (DockButton* button : mButtons)
        {
            if (button->widget() == widget)
            {
                button->setChecked(true);
                break;
            }
        }
    }

    void DockBar::uncheckButton(ContentWidget* widget)
    {
        for (DockButton* button : mButtons)
        {
            if (button->widget() == widget)
            {
                button->setChecked(false);
                break;
            }
        }
    }

    ContentWidget* DockBar::widgetAt(int index)
    {
        if (0 <= index && index < mButtons.size())
            return mButtons.at(index)->widget();
        return nullptr;
    }

    ContentWidget* DockBar::nextAvailableWidget(int index)
    {
        if (index < 0)
        {
            for (DockButton* button : mButtons)
            {
                if (!button->hidden())
                    return button->widget();
            }
            return nullptr;
        }
        if (index >= mButtons.size())
        {
            ContentWidget* widget = nullptr;
            for (DockButton* button : mButtons)
            {
                if (!button->hidden())
                    widget = button->widget();
            }
            return widget;
        }
        int current_index      = 0;
        ContentWidget* widget = nullptr;
        for (DockButton* button : mButtons)
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

    void DockBar::handleDragStart()
    {
        show();
    }

    void DockBar::handleDragEnd()
    {
        if (mAutohide && unused())
            hide();
    }
}
