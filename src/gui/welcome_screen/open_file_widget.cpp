#include "welcome_screen/open_file_widget.h"

#include "file_manager/file_manager.h"
#include "gui_utils/graphics.h"

#include <QDragEnterEvent>
#include <QLabel>
#include <QMimeData>
#include <QStyle>
#include <QVBoxLayout>

namespace hal
{
    open_file_widget::open_file_widget(QWidget* parent) : QFrame(parent), m_layout(new QVBoxLayout()), m_text_label(new QLabel()), m_icon_label(new QLabel()), m_drag_active(false)
    {
        setAcceptDrops(true);

        m_text_label->setObjectName("text-label");
        m_text_label->setText("Open a new file using the toolbar or simply drag & drop one here");
        m_text_label->setWordWrap(true);

        m_icon_label->setObjectName("icon-label");
        m_icon_label->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        m_icon_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        setLayout(m_layout);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);
        m_layout->addWidget(m_text_label);
        m_layout->addWidget(m_icon_label);
    }

    void open_file_widget::dragEnterEvent(QDragEnterEvent* event)
    {
        const QMimeData* mime_data = event->mimeData();
        if (mime_data->hasUrls())
        {
            event->acceptProposedAction();
            m_drag_active = true;
            repolish();
        }
    }

    void open_file_widget::dragLeaveEvent(QDragLeaveEvent* event)
    {
        Q_UNUSED(event)

        m_drag_active = false;
        repolish();
    }

    void open_file_widget::dropEvent(QDropEvent* event)
    {
        // pass url list to file manager and check for matching file extensions,
        //if match found -> show progress screen else do nothing
        const QMimeData* mime_data = event->mimeData();
        if (mime_data->hasUrls())
        {
            //        QStringList path_list;
            //        QList<QUrl> url_list = mime_data->urls();

            //        if (url_list.size() > 1)
            //            return;

            QString file = mime_data->urls().at(0).toLocalFile();
            file_manager::get_instance()->open_file(file);

            //        for (int i = 0; i < url_list.size() && i < 32; ++i)
            //        {
            //            path_list.append(url_list.at(i).toLocalFile());
            //        }

            event->acceptProposedAction();
            m_drag_active = false;
            repolish();
        }
    }

    bool open_file_widget::drag_active()
    {
        return m_drag_active;
    }

    QString open_file_widget::icon_style()
    {
        return m_icon_style;
    }

    QString open_file_widget::icon_path()
    {
        return m_icon_path;
    }

    void open_file_widget::set_icon_style(const QString& style)
    {
        m_icon_style = style;
    }

    void open_file_widget::set_icon_path(const QString& path)
    {
        m_icon_path = path;
    }

    void open_file_widget::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        if (!m_icon_path.isEmpty())
            m_icon_label->setPixmap(gui_utility::get_styled_svg_icon(m_icon_style, m_icon_path).pixmap(QSize(120, 120)));
    }
}
