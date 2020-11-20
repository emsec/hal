#include "gui/style/style_manager.h"

#include "hal_core/utilities/log.h"

#include "gui/graph_widget/graphics_qss_adapter.h"
#include "gui/code_editor/syntax_highlighter/python_qss_adapter.h"
#include "gui/code_editor/syntax_highlighter/vhdl_qss_adapter.h"
#include "gui/notifications/notification_manager_qss_adapter.h"
#include "gui/style/shared_properties_qss_adapter.h"
#include "gui/gui_globals.h"

#include <QApplication>
#include <QFile>

namespace hal
{
    StyleManager::StyleManager(QObject* parent)
        : QObject(parent), mGraphicsQssAdapter(new GraphicsQssAdapter()), mSharedProperties(new SharedPropertiesQssAdapter()),
          mNotificationManager(new NotificationManagerQssAdapter()), mPythonSyntaxHighlighter(new PythonQssAdapter()), mVhdlSyntaxHighlighter(new VhdlQssAdapter())
    {
    }

    StyleManager* StyleManager::get_instance()
    {
        static StyleManager instance;
        return &instance;
    }

    StyleManager::~StyleManager()
    {
        // QT DELETES WIDGETS AUTOMATICALLY NOT SURE IF NEEDED, CHECK
        // POSSIBLY USEFUL TO ENSURE DESTRUCTION ORDER
    }

    void StyleManager::updateStyle()
    {
        QCoreApplication* core = QApplication::instance();
        QApplication* app      = qobject_cast<QApplication*>(core);

        if (!app)
            return;

        QString path = gSettingsManager->get("stylesheet/base").toString();
        QFile stylesheet(path);

        if (!stylesheet.exists())
        {
            log_error("gui", "Specified stylesheet '{}' does not exist, proceeding with default style", path.toStdString());
            setDefault(app);
            return;
        }

        if (!stylesheet.open(QFile::ReadOnly))
        {
            log_error("gui", "Unable to open specified stylesheet '{}', proceeding with default style", path.toStdString());
            setDefault(app);
            return;
        }

        path = gSettingsManager->get("stylesheet/definitions").toString();

        if (path.isEmpty())
        {
            app->setStyleSheet(QString(stylesheet.readAll()));
            stylesheet.close();
            return;
        }

        QFile definitions(path);

        if (!definitions.exists())
        {
            log_error("gui", "Specified stylesheet definitions file '{}' does not exist, proceeding with default style", path.toStdString());
            setDefault(app);
            return;
        }

        if (!definitions.open(QFile::ReadOnly))
        {
            log_error("gui", "Unable to open specified stylesheet definitions file '{}', proceeding with default style", path.toStdString());
            setDefault(app);
            return;
        }

        // COMBINE BASE AND DEFINITIONS INTO FINAL STYLESHEET
    }

    const SharedPropertiesQssAdapter* StyleManager::sharedProperties() const
    {
        return mSharedProperties;
    }

    const NotificationManagerQssAdapter* StyleManager::NotificationManager() const
    {
        return mNotificationManager;
    }

    const PythonQssAdapter* StyleManager::PythonSyntaxHighlighter() const
    {
        return mPythonSyntaxHighlighter;
    }

    const VhdlQssAdapter* StyleManager::VhdlSyntaxHighlighter() const
    {
        return mVhdlSyntaxHighlighter;
    }

    void StyleManager::setDefault(QApplication* app)
    {
        QFile fallback(":/style/hal");
        fallback.open(QFile::ReadOnly);
        app->setStyleSheet(QString(fallback.readAll()));
        fallback.close();
    }
}
