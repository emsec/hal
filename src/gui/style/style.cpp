#include "style/style.h"

#include "core/log.h"

#include "gui_globals.h"

#include <QApplication>
#include <QFile>
#include <QHash>
#include <QRegularExpression>
#include <QTextStream>

namespace style
{
    QString default_stylesheet()
    {
        QFile file(":/style/hal");
        file.open(QFile::ReadOnly);
        QString stylesheet = QString(file.readAll());
        file.close();    // NOT SURE IF NECESSARY
        return stylesheet;
    }

    QString get_stylesheet()
    {
        QString stylesheet = "";
        {
            QString path = g_settings.value("stylesheet/base").toString();
            QFile file(path);

            if (!file.exists())
            {
                log_error("gui", "Specified stylesheet '{}' does not exist, proceeding with default style", path.toStdString());
                return default_stylesheet();
            }

            if (!file.open(QFile::ReadOnly))
            {
                log_error("gui", "Unable to open specified stylesheet '{}', proceeding with default style", path.toStdString());
                return default_stylesheet();
            }

            stylesheet = QString(file.readAll());
            file.close();
        }

        QString path = g_settings.value("stylesheet/definitions").toString();

        if (path.isEmpty())
            return stylesheet;

        QHash<QString, QString> definitions;
        QRegularExpression key_regex("(?<key>@[a-zA-Z0-9_]+)($|[^a-zA-Z0-9_])");
        key_regex.optimize();
        {
            QFile file(path);

            if (!file.exists())
            {
                log_error("gui", "Specified stylesheet definitions file '{}' does not exist, proceeding with default style", path.toStdString());
                return default_stylesheet();
            }

            if (!file.open(QFile::ReadOnly))
            {
                log_error("gui", "Unable to open specified stylesheet definitions file '{}', proceeding with default style", path.toStdString());
                return default_stylesheet();
            }

            QRegularExpression value_regex("^\\s*(?<value>\\S[^;]*);");
            value_regex.optimize();

            QTextStream in(&file);
            while (!in.atEnd())
            {
                QString line = in.readLine();

                if (line.isEmpty())
                    continue;

                int index          = line.indexOf(':');
                QString key_part   = line.left(index);
                QString value_part = line.remove(0, index + 1);

                key_part.replace(',', ' ');
                QString value = value_regex.match(value_part).captured("value");

                QRegularExpressionMatchIterator it = key_regex.globalMatch(key_part);
                while (it.hasNext())
                {
                    QRegularExpressionMatch match = it.next();
                    definitions.insert(match.captured("key"), value);
                }
            }
            file.close();
        }

        QRegularExpressionMatchIterator it = key_regex.globalMatch(stylesheet);
        int offset                         = 0;

        while (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();

            QString key   = match.captured("key");
            QString value = definitions.value(key);

            int index  = match.capturedStart() + offset;
            int length = key.length();

            stylesheet.remove(index, length);
            stylesheet.insert(index, value);

            offset = offset - key.length() + value.length();
        }

        return stylesheet;
    }
}    // namespace style
