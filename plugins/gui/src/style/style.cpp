#include "gui/style/style.h"

#include "hal_core/utilities/log.h"

#include "gui/graph_widget/items/graphics_item.h"
#include "gui/graph_widget/items/nets/graphics_net.h"
#include "gui/graph_widget/items/nodes/gates/standard_graphics_gate.h"
#include "gui/graph_widget/items/nodes/modules/standard_graphics_module.h"
#include "gui/graph_widget/items/nets/arrow_separated_net.h"
#include "gui/graph_widget/items/nets/circle_separated_net.h"
#include "gui/graph_widget/items/nets/labeled_separated_net.h"
#include "gui/graph_widget/items/nets/separated_graphics_net.h"
#include "gui/graph_widget/items/nets/standard_graphics_net.h"
#include "gui/graph_widget/items/utility_items/node_drag_shadow.h"
#include "gui/gui_globals.h"
#include "gui/svg_icon_engine/svg_icon_engine.h"

#include <QApplication>
#include <QFile>
#include <QHash>
#include <QRegularExpression>
#include <QTextStream>

namespace hal
{
    namespace style
    {
        QIcon getStyledSvgIcon(const QString& from_to_colors, const QString& svg_path)
        {
            QString svg_data;
            QFile file(svg_path);

            if (file.open(QFile::ReadOnly))
                svg_data = QString(file.readAll());
            else
            {
                //log_error("error");
                return QIcon();
            }

            if (svg_data.isEmpty())
            {
                //log_error("error");
                return QIcon();
            }

            if (!from_to_colors.isEmpty())
            {
                QRegExp color_regex("#[0-9a-f]{6}", Qt::CaseInsensitive);
                QRegExp all_to_color_regex("\\s*all\\s*->\\s*#[0-9a-f]{6}\\s*", Qt::CaseInsensitive);
                QRegExp color_to_color_regex("\\s*(#[0-9a-f]{6}\\s*->\\s*#[0-9a-f]{6}\\s*,\\s*)*#[0-9a-f]{6}\\s*->\\s*#[0-9a-f]{6}\\s*", Qt::CaseInsensitive);
                if (all_to_color_regex.exactMatch(from_to_colors))
                {
                    color_regex.indexIn(from_to_colors);
                    QString color = color_regex.cap(0);
                    svg_data.replace(color_regex, color.toUtf8());
                }
                else if (color_to_color_regex.exactMatch(from_to_colors))
                {
                    QString copy = from_to_colors;
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
                    //print error
                    return QIcon();
                }
            }

            return QIcon(new SvgIconEngine(svg_data.toStdString()));
        }

        void debugUpdate()
        {
            GraphicsItem::loadSettings();
            StandardGraphicsModule::loadSettings();
            StandardGraphicsGate::loadSettings();
            GraphicsNet::loadSettings();
            StandardGraphicsNet::loadSettings();
            SeparatedGraphicsNet::loadSettings();
            ArrowSeparatedNet::loadSettings();
            CircleSeparatedNet::loadSettings();
            LabeledSeparatedNet::loadSettings();
            NodeDragShadow::loadSettings();
        }

    } // namespace style
}
