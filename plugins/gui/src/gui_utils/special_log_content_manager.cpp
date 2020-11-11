#include "gui/gui_utils/special_log_content_manager.h"
#include "gui/main_window/main_window.h"
#include "gui/python/python_editor.h"
#include "gui/file_manager/file_manager.h"

#include <QTimer>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QTabWidget>
#include <QPlainTextEdit> //the python code editor
#include <QWindow>
#include <QWindowList>
#include <QPainter>

namespace hal
{
    SpecialLogContentManager::SpecialLogContentManager(QObject* parent) : QObject(parent), m_timer(new QTimer(this))
    {

    }

    SpecialLogContentManager::SpecialLogContentManager(MainWindow *parent, PythonEditor *python_editor) : QObject(parent), m_main_window(parent),
       m_timer(new QTimer(this)), m_python_editor(python_editor)
    {
        m_screenshot_sub_path = "/screenshots";
        m_python_content_sub_path = "/pythoneditors";

        connect(m_timer, &QTimer::timeout, this, &SpecialLogContentManager::safe_screenshot);
        connect(m_timer, &QTimer::timeout, this, &SpecialLogContentManager::safe_python_editor);
    }

    SpecialLogContentManager::~SpecialLogContentManager()
    {

    }

    void SpecialLogContentManager::safe_screenshot()
    {
        QString hal_file_name = FileManager::get_instance()->file_name();

        if(hal_file_name.isEmpty() || qGuiApp->topLevelWindows().isEmpty())
            return;

        QDir hal_file_dir = QFileInfo(hal_file_name).absoluteDir();
        QString hal_file_name_sub_path = "/" + QFileInfo(hal_file_name).baseName();
        QString screenshot_path = hal_file_dir.path() + hal_file_name_sub_path + m_screenshot_sub_path;
        if(!hal_file_dir.exists(screenshot_path))
        {
            if(!hal_file_dir.mkpath(screenshot_path))
            {
                qDebug() << "Failed to create screenshots directory.";
                return;
            }
        }

        QString file_name = "Screenshot_" +QString::number(QDateTime::currentDateTime().toTime_t());
        QString file_type = "png";

        QList<QPixmap> pixmap_list;
        int total_width = 0;
        int max_height = 0;
        for(int i = 0; i < qGuiApp->topLevelWindows().size(); i++)
        {
            QWidget* found_wid = QWidget::find(qGuiApp->topLevelWindows().at(i)->winId());
            if(found_wid)
            {
                QPixmap p(found_wid->size());
                found_wid->render(&p);
                total_width += p.width();
                max_height = (max_height > p.height()) ? max_height : p.height();
                pixmap_list.append(p);

            }
            else
                qDebug() << "Could not find any top level widget to screenshot.";
        }

        QImage image(total_width, max_height, QImage::Format_RGB32);
        QPainter painter(&image);
        int curr_x = 0;
        for(int i = 0; i < pixmap_list.size(); i++)
        {
            QPixmap curr_pixmap = pixmap_list.at(i);
            painter.drawPixmap(curr_x, 0, curr_pixmap);
            painter.fillRect(curr_x, curr_pixmap.height(), curr_pixmap.width(), max_height-curr_pixmap.height(),Qt::white);
            curr_x += curr_pixmap.width();
        }

        if(!image.save(screenshot_path + "/" + file_name + "." + file_type))
            qDebug() << "Could not save image!";
    }

    void SpecialLogContentManager::safe_python_editor()
    {
        QString hal_file_name = FileManager::get_instance()->file_name();

        if(!m_python_editor || !m_python_editor->get_tab_widget() || hal_file_name.isEmpty())
            return;

        QDir hal_file_dir = QFileInfo(hal_file_name).absoluteDir();
        QString hal_file_name_sub_path = "/" + QFileInfo(hal_file_name).baseName();
        QString python_editor_content_dumb_path = hal_file_dir.path() + hal_file_name_sub_path + m_python_content_sub_path;
        if(!hal_file_dir.exists(python_editor_content_dumb_path))
        {
            if(!hal_file_dir.mkpath(python_editor_content_dumb_path))
            {
                qDebug() << "Failed to create python editor dumb directory.";
                return;
            }
        }

        QTabWidget* python_tab_widget = m_python_editor->get_tab_widget();
        QString file_name = "Pythoncodeeditors_" + QString::number(QDateTime::currentDateTime().toTime_t());
        QString file_type = "txt";
        QFile file(python_editor_content_dumb_path + "/" + file_name + "." + file_type);

        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << "Could not open file with path: " << file.fileName() << ". Abort.";
            return;
        }

        QTextStream ds(&file);
        ds << "Number of tabs: " << QString::number(python_tab_widget->count()) << "\n";

        for(int i = 0; i < python_tab_widget->count(); i++)
        {
            QPlainTextEdit* python_editor = dynamic_cast<QPlainTextEdit*>(python_tab_widget->widget(i));
            QString content = "";

            if(python_editor)
                content = python_editor->toPlainText();
            else
                content = "Could not get python code editor content.";

            ds << "---------------------Start of new tab------------------------\nTabnumber: " << i
               << "\nName: "<< python_tab_widget->tabText(i) << "\nContent:\n" << content << "\n";

        }
        file.close();
    }

    void SpecialLogContentManager::start_logging(int msec)
    {
        m_timer->start(msec);
    }

}
