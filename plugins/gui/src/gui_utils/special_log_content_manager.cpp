#include "gui/gui_utils/special_log_content_manager.h"
#include "gui/main_window/main_window.h"
#include "gui/python/python_editor.h"

#include <QTimer>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>

namespace hal
{
    SpecialLogContentManager::SpecialLogContentManager(QObject* parent) : QObject(parent), m_timer(new QTimer(this))
    {

    }

    SpecialLogContentManager::SpecialLogContentManager(MainWindow *parent, PythonEditor *python_editor) : QObject(parent), m_main_window(parent),
       m_timer(new QTimer(this)), m_python_editor(python_editor)
    {
        m_screenshot_path = "/home/sebbe/Desktop/LOGTESTS";
        m_python_content_path = "/home/sebbe/Desktop/LOGTESTS";

        //QObject::connect(m_timer, &QTimer::timeout, this, &SpecialLogContentManager::safe_screenshot);
        connect(m_timer, &QTimer::timeout, this, &SpecialLogContentManager::safe_screenshot);
    }

    SpecialLogContentManager::~SpecialLogContentManager()
    {

    }

    void SpecialLogContentManager::safe_screenshot()
    {
        //qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy hh-mm-ss");
        QDir dir(QDir::currentPath());
        if(!dir.exists(dir.path() + "/LUBADA"))
        {
            if(!dir.mkdir("LUBADA"))
            {
                qDebug() << "Failed to create directory to log, abort mission! I repeat, abort mission!";
                return;
            }
        }
        //variant with grabWidget does not seem to work, only hal screenshot
//        QPixmap pixmap(m_main_window->size());
//        m_main_window->render(&pixmap);
        //variant to take a screenshot of entire screen
        QPixmap pixmap = QGuiApplication::primaryScreen()->grabWindow(0);

        QString file_name = "Screenshot_" + QDateTime::currentDateTime().toString("dd-MM-yyyy_hh-mm-ss");
        QString file_type = "png";
        QString file_path = dir.currentPath() + "/LUBADA";
        if(!pixmap.save(file_path + "/" + file_name + "." + file_type))
            qDebug() << "could not save the screenshot";

    }

    void SpecialLogContentManager::safe_python_editor()
    {
        qDebug() << "SAVING EDITOR";
    }

    void SpecialLogContentManager::start_logging(int msec)
    {
        m_timer->start(msec);
    }

}
