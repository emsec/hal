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
    SpecialLogContentManager::SpecialLogContentManager(QObject* parent) : QObject(parent), mTimer(new QTimer(this))
    {

    }

    SpecialLogContentManager::SpecialLogContentManager(QObject *parent, PythonEditor *python_editor) : QObject(parent),
       mTimer(new QTimer(this)), mPythonEditor(python_editor)
    {
        mScreenshotSubPath = "/screenshots";
        mPythonContentSubPath = "/pythoneditors";

        connect(mTimer, &QTimer::timeout, this, &SpecialLogContentManager::safeScreenshot);
        connect(mTimer, &QTimer::timeout, this, &SpecialLogContentManager::safePythonEditor);
    }

    SpecialLogContentManager::~SpecialLogContentManager()
    {

    }

    void SpecialLogContentManager::safeScreenshot()
    {
        QString halFileName = FileManager::get_instance()->fileName();

        if(halFileName.isEmpty() || qGuiApp->topLevelWindows().isEmpty())
            return;

        QDir halFileDir = QFileInfo(halFileName).absoluteDir();
        QString halFileNameSubPath = "/" + QFileInfo(halFileName).baseName();
        QString screenshotPath = halFileDir.path() + halFileNameSubPath + mScreenshotSubPath;
        if(!halFileDir.exists(screenshotPath))
        {
            if(!halFileDir.mkpath(screenshotPath))
            {
                qDebug() << "Failed to create screenshots directory.";
                return;
            }
        }

        QString fileName = "Screenshot_" +QString::number(QDateTime::currentDateTime().toTime_t());
        QString fileType = "png";

        QList<QPixmap> pixmapList;
        int totalWidth = 0;
        int maxHeight = 0;
        for(int i = 0; i < qGuiApp->topLevelWindows().size(); i++)
        {
            QWidget* found_wid = QWidget::find(qGuiApp->topLevelWindows().at(i)->winId());
            if(found_wid)
            {
                QPixmap p(found_wid->size());
                found_wid->render(&p);
                totalWidth += p.width();
                maxHeight = (maxHeight > p.height()) ? maxHeight : p.height();
                pixmapList.append(p);
            }
        }

        QImage image(totalWidth, maxHeight, QImage::Format_RGB32);
        QPainter painter(&image);
        int curr_x = 0;
        for(int i = 0; i < pixmapList.size(); i++)
        {
            QPixmap currPixmap = pixmapList.at(i);
            painter.drawPixmap(curr_x, 0, currPixmap);
            painter.fillRect(curr_x, currPixmap.height(), currPixmap.width(), maxHeight-currPixmap.height(),Qt::white);
            curr_x += currPixmap.width();
        }

        if(!image.save(screenshotPath + "/" + fileName + "." + fileType))
            qDebug() << "Could not save image!";
    }

    void SpecialLogContentManager::safePythonEditor()
    {
        QString halFileName = FileManager::get_instance()->fileName();

        if(!mPythonEditor || !mPythonEditor->getTabWidget() || halFileName.isEmpty())
            return;

        QDir halFileDir = QFileInfo(halFileName).absoluteDir();
        QString halFileNameSubPath = "/" + QFileInfo(halFileName).baseName();
        QString pythonEditorDumpPath = halFileDir.path() + halFileNameSubPath + mPythonContentSubPath;
        if(!halFileDir.exists(pythonEditorDumpPath))
        {
            if(!halFileDir.mkpath(pythonEditorDumpPath))
            {
                qDebug() << "Failed to create python editor dumb directory.";
                return;
            }
        }

        QTabWidget* pythonTabWidget = mPythonEditor->getTabWidget();
        QString fileName = "Pythoncodeeditors_" + QString::number(QDateTime::currentDateTime().toTime_t());
        QString fileType = "txt";
        QFile file(pythonEditorDumpPath + "/" + fileName + "." + fileType);

        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << "Could not open file with path: " << file.fileName() << ". Abort.";
            return;
        }

        QTextStream textStream(&file);
        textStream << "Number of tabs: " << QString::number(pythonTabWidget->count()) << "\n";

        for(int i = 0; i < pythonTabWidget->count(); i++)
        {
            QPlainTextEdit* python_editor = dynamic_cast<QPlainTextEdit*>(pythonTabWidget->widget(i));
            QString content = "";

            if(python_editor)
                content = python_editor->toPlainText();
            else
                content = "Could not get python code editor content.";

            textStream << "---------------------Start of new tab------------------------\nTabnumber: " << i
                       << "\nName: "<< pythonTabWidget->tabText(i) << "\nContent:\n" << content << "\n";

        }
        file.close();
    }

    void SpecialLogContentManager::startLogging(int msec)
    {
        mTimer->start(msec);
    }

}
