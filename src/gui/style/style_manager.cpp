#include "style/style_manager.h"

#include "core/log.h"

#include "graph_widget/graphics_qss_adapter.h"
#include "gui/code_editor/syntax_highlighter/python_qss_adapter.h"
#include "gui/code_editor/syntax_highlighter/vhdl_qss_adapter.h"
#include "gui/notifications/notification_manager_qss_adapter.h"
#include "gui/style/shared_properties_qss_adapter.h"
#include "gui_globals.h"

#include <QApplication>
#include <QFile>

namespace hal
{
    style_manager::style_manager(QObject* parent)
        : QObject(parent), m_graphics_qss_adapter(new GraphicsQssAdapter()), m_shared_properties(new shared_properties_qss_adapter()),
          m_NotificationManager(new NotificationManagerQssAdapter()), m_python_syntax_highlighter(new PythonQssAdapter()), m_vhdl_syntax_highlighter(new VhdlQssAdapter())
    {
    }

    style_manager* style_manager::get_instance()
    {
        static style_manager instance;
        return &instance;
    }

    style_manager::~style_manager()
    {
        // QT DELETES WIDGETS AUTOMATICALLY NOT SURE IF NEEDED, CHECK
        // POSSIBLY USEFUL TO ENSURE DESTRUCTION ORDER
    }

    void style_manager::update_style()
    {
        QCoreApplication* core = QApplication::instance();
        QApplication* app      = qobject_cast<QApplication*>(core);

        if (!app)
            return;

        QString path = g_settings_manager.get("stylesheet/base").toString();
        QFile stylesheet(path);

        if (!stylesheet.exists())
        {
            log_error("gui", "Specified stylesheet '{}' does not exist, proceeding with default style", path.toStdString());
            set_default(app);
            return;
        }

        if (!stylesheet.open(QFile::ReadOnly))
        {
            log_error("gui", "Unable to open specified stylesheet '{}', proceeding with default style", path.toStdString());
            set_default(app);
            return;
        }

        path = g_settings_manager.get("stylesheet/definitions").toString();

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
            set_default(app);
            return;
        }

        if (!definitions.open(QFile::ReadOnly))
        {
            log_error("gui", "Unable to open specified stylesheet definitions file '{}', proceeding with default style", path.toStdString());
            set_default(app);
            return;
        }

        // COMBINE BASE AND DEFINITIONS INTO FINAL STYLESHEET
    }

    const shared_properties_qss_adapter* style_manager::shared_properties() const
    {
        return m_shared_properties;
    }

    const NotificationManagerQssAdapter* style_manager::NotificationManager() const
    {
        return m_NotificationManager;
    }

    const PythonQssAdapter* style_manager::PythonSyntaxHighlighter() const
    {
        return m_python_syntax_highlighter;
    }

    const VhdlQssAdapter* style_manager::VhdlSyntaxHighlighter() const
    {
        return m_vhdl_syntax_highlighter;
    }

    void style_manager::set_default(QApplication* app)
    {
        QFile fallback(":/style/hal");
        fallback.open(QFile::ReadOnly);
        app->setStyleSheet(QString(fallback.readAll()));
        fallback.close();
    }
}
