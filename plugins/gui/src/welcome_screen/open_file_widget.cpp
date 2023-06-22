#include "gui/welcome_screen/open_file_widget.h"

#include "gui/file_manager/file_manager.h"
#include "gui/gui_utils/graphics.h"
#include "hal_core/utilities/log.h"

#include <QDragEnterEvent>
#include <QLabel>
#include <QMimeData>
#include <QStyle>
#include <QVBoxLayout>

namespace hal
{
    OpenFileWidget::OpenFileWidget(QWidget* parent) : QFrame(parent), mLayout(new QVBoxLayout()), mTextLabel(new QLabel()), mIconLabel(new QLabel()), mDragActive(false)
    {
        setAcceptDrops(true);

        mTextLabel->setObjectName("text-label");
        mTextLabel->setText("Open an existing project or create a new one by dropping a project folder or netlist file here.");
        mTextLabel->setWordWrap(true);

        mIconLabel->setObjectName("icon-label");
        mIconLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        mIconLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        setLayout(mLayout);
        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);
        mLayout->addWidget(mTextLabel);
        mLayout->addWidget(mIconLabel);
    }

    void OpenFileWidget::dragEnterEvent(QDragEnterEvent* event)
    {
        const QMimeData* mime_data = event->mimeData();
        if (mime_data->hasUrls())
        {
            event->acceptProposedAction();
            mDragActive = true;
            repolish();
        }
    }

    void OpenFileWidget::dragLeaveEvent(QDragLeaveEvent* event)
    {
        Q_UNUSED(event)

        mDragActive = false;
        repolish();
    }

    void OpenFileWidget::dropEvent(QDropEvent* event)
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

            QString file                      = mime_data->urls().at(0).toLocalFile();
            FileManager::DirectoryStatus stat = FileManager::directoryStatus(file);

            switch (stat)
            {
                case FileManager::ProjectDirectory:
                    FileManager::get_instance()->openProject(file);
                    break;
                case FileManager::OtherDirectory:
                    log_warning("gui", "directory {} is not a hal project.", file.toStdString());
                    break;
                case FileManager::IsFile:
                    FileManager::get_instance()->importFile(file);
                    break;
                default:
                    log_warning("gui", "cannot open nor import {}.", file.toStdString());
                    break;
            }

            //        for (int i = 0; i < url_list.size() && i < 32; ++i)
            //        {
            //            path_list.append(url_list.at(i).toLocalFile());
            //        }

            event->acceptProposedAction();
            mDragActive = false;
            repolish();
        }
    }

    bool OpenFileWidget::dragActive()
    {
        return mDragActive;
    }

    QString OpenFileWidget::iconStyle()
    {
        return mIconStyle;
    }

    QString OpenFileWidget::iconPath()
    {
        return mIconPath;
    }

    void OpenFileWidget::setIconStyle(const QString& style)
    {
        mIconStyle = style;
    }

    void OpenFileWidget::setIconPath(const QString& path)
    {
        mIconPath = path;
    }

    void OpenFileWidget::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        if (!mIconPath.isEmpty())
            mIconLabel->setPixmap(gui_utility::getStyledSvgIcon(mIconStyle, mIconPath).pixmap(QSize(120, 120)));
    }
}    // namespace hal
