#include "gui/gui_utils/graphics.h"

#include "gui/svg_icon_engine/svg_icon_engine.h"

#include <QFile>
#include <QImage>

namespace hal
{
    namespace gui_utility
    {
        void changeSvgColor(QString& svg_data, const QColor& from, const QColor& to)
        {
            QRegExp regex(from.name());
            svg_data.replace(regex, to.name().toUtf8());
        }

        void changeAllSvgColors(QString& svg_data, const QColor& to)
        {
            QRegExp regex("#[0-9a-f]{6}", Qt::CaseInsensitive);
            svg_data.replace(regex, to.name().toUtf8());
        }

        bool applyColorStyle(QString& svg_data, const QString& color_style)
        {
            QRegExp color_regex("#[0-9a-f]{6}", Qt::CaseInsensitive);
            QRegExp all_to_color_regex("\\s*all\\s*->\\s*#[0-9a-f]{6}\\s*", Qt::CaseInsensitive);
            QRegExp color_to_color_regex("\\s*(#[0-9a-f]{6}\\s*->\\s*#[0-9a-f]{6}\\s*,\\s*)*#[0-9a-f]{6}\\s*->\\s*#[0-9a-f]{6}\\s*", Qt::CaseInsensitive);
            if (all_to_color_regex.exactMatch(color_style))
            {
                color_regex.indexIn(color_style);
                QString color = color_regex.cap(0);
                svg_data.replace(color_regex, color.toUtf8());
            }
            else if (color_to_color_regex.exactMatch(color_style))
            {
                QString copy = color_style;
                copy         = copy.simplified();
                copy.replace(" ", "");

                QStringList list = copy.split(",");

                for (QString string : list)
                {
                    QString from_color = string.left(7);
                    QString to_color   = string.right(7);

                    QRegExp regex(from_color);
                    svg_data.replace(regex, to_color.toUtf8());
                }
            }
            else
            {
                return false;
            }
            return true;
        }


        QIcon getStyledSvgIcon(const QString& from_to_colors_enabled, const QString& svg_path, QString from_to_colors_disabled)
        {
            QFile file(svg_path);
            if (!file.open(QFile::ReadOnly))
            {
                //TODO: print error
                return QIcon();
            }

            QString svgDataEnabled(file.readAll());
            if (svgDataEnabled.isEmpty())
            {
                //TODO: print error
                return QIcon();
            }

            QString svgDataDisabled(svgDataEnabled);

            if (!from_to_colors_enabled.isEmpty())
            {
                if (!applyColorStyle(svgDataEnabled,from_to_colors_enabled))
                {
                    //TODO: print error
                    return QIcon();
                }
            }

            if (from_to_colors_disabled.isEmpty())
            {
                svgDataDisabled = QString();
            }
            else
            {
                if (!applyColorStyle(svgDataDisabled,from_to_colors_disabled))
                    svgDataDisabled = QString();
            }

            return QIcon(new SvgIconEngine(svgDataEnabled.toUtf8(),svgDataDisabled.toUtf8()));
        }
    }    // namespace gui_utility
}
