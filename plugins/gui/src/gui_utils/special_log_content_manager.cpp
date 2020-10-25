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
#include <QTabWidget>
#include <QPlainTextEdit> //the python code editor

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
        connect(m_timer, &QTimer::timeout, this, &SpecialLogContentManager::safe_python_editor);
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
                qDebug() << "Failed to create directory to log screenshots, abort mission! I repeat, abort mission!";
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
        if(!m_python_editor || !m_python_editor->get_tab_widget())
            return;

        QTabWidget* python_tab_widget = m_python_editor->get_tab_widget();
        QDir dir(QDir::currentPath());
        if(!dir.exists(dir.path() + "/RALDAS"))
        {
            if(!dir.mkdir("RALDAS"))
            {
                qDebug() << "Failed to create directory to log python-code-editors, abort mission! I repeat, abort mission!";
                return;
            }
        }

        QString file_name = "Pythoncodeeditors_" + QDateTime::currentDateTime().toString("dd-MM-yyyy_hh-mm-ss");
        QString file_type = "txt";
        QString file_path = dir.currentPath() + "/RALDAS";
        QFile file(file_path + "/" + file_name + "." + file_type);

        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << "Could not open file with path: " << file.fileName() << " in safe_python_editor. Abort.";
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
