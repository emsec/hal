#include "style/style_manager.h"

#include "core/log.h"

#include "gui_globals.h"

#include <QApplication>
#include <QFile>

style_manager::style_manager(QObject* parent) : QObject(parent)
{
}

void style_manager::update_style()
{
    QCoreApplication* core = QApplication::instance();
    QApplication* app      = qobject_cast<QApplication*>(core);

    if (!app)
        return;

    QString path = g_settings.value("stylesheet/base").toString();
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

    path = g_settings.value("stylesheet/definitions").toString();

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

void style_manager::set_default(QApplication* app)
{
    QFile fallback(":/style/hal");
    fallback.open(QFile::ReadOnly);
    app->setStyleSheet(QString(fallback.readAll()));
    fallback.close();
}
