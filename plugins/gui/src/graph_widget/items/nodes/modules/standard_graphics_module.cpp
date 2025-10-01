#include "gui/graph_widget/items/nodes/modules/standard_graphics_module.h"

#include "gui/graph_widget/graph_widget_constants.h"
#include "gui/graph_widget/graphics_qss_adapter.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"
#include "hal_core/netlist/gate.h"

#include <QImage>
#include <QPainter>
#include <QPen>
#include <QStyle>
#include <QStyleOptionGraphicsItem>

namespace hal
{
    static const qreal sBaseline = 1;

    qreal StandardGraphicsModule::sAlpha;

    QPen StandardGraphicsModule::sPen;

    QFont StandardGraphicsModule::sPinFont;

    qreal StandardGraphicsModule::sColorBarHeight = 30;

    qreal StandardGraphicsModule::sPinInnerHorizontalSpacing = 12;
    qreal StandardGraphicsModule::sPinOuterHorizontalSpacing = 2.4;

    qreal StandardGraphicsModule::sPinInnerVerticalSpacing = 1.2;
    qreal StandardGraphicsModule::sPinOuterVerticalSpacing = 0.6;
    qreal StandardGraphicsModule::sPinUpperVerticalSpacing = 0.6;
    qreal StandardGraphicsModule::sPinLowerVerticalSpacing = 1.8;

    qreal StandardGraphicsModule::sPinFontHeight;
    qreal StandardGraphicsModule::sPinFontAscent;
    qreal StandardGraphicsModule::sPinFontDescent;
    qreal StandardGraphicsModule::sPinFontBaseline;

    qreal StandardGraphicsModule::sInnerNameTypeSpacing = 1.2;
    qreal StandardGraphicsModule::sOuterNameTypeSpacing = 3;

    const int StandardGraphicsModule::sIconPadding = 3;
    const QSize StandardGraphicsModule::sIconSize(sColorBarHeight - 2 * sIconPadding, sColorBarHeight - 2 * sIconPadding);

    void StandardGraphicsModule::loadSettings()
    {
        sPen.setCosmetic(true);
        sPen.setJoinStyle(Qt::MiterJoin);

        sTextColor = GraphicsQssAdapter::instance()->nodeTextColor();

        QFont font = QFont("Iosevka");
        font.setPixelSize(graph_widget_constants::sFontSize);

        sPinFont = font;

        QFontMetricsF pin_fm(sPinFont);
        sPinFontHeight   = pin_fm.height();
        sPinFontAscent   = pin_fm.ascent();
        sPinFontDescent  = pin_fm.descent();
        sPinFontBaseline = 1;

        sSelectionColor = QColor(240, 173, 0);
        sHighlightColor = QColor(40, 200, 240);
    }

    void StandardGraphicsModule::updateAlpha()
    {
        if (sLod <= graph_widget_constants::sGateMaxLod)
            sAlpha = 1 - (sLod - graph_widget_constants::sGateMinLod) / (graph_widget_constants::sGateMaxLod - graph_widget_constants::sGateMinLod);
        else
            sAlpha = 0;
    }

    StandardGraphicsModule::StandardGraphicsModule(Module* m, bool adjust_size_to_grid) : GraphicsModule(m)
    {
        format(adjust_size_to_grid);
    }

    void StandardGraphicsModule::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
    {
        Q_UNUSED(widget);

        if (sLod < graph_widget_constants::sGateMinLod)
        {
            painter->fillRect(QRectF(0, 0, mWidth, mHeight), penColor(option->state));
            return;
        }
        else
        {
            // draw box and icon
            painter->fillRect(QRectF(0, 0, mWidth, sColorBarHeight), mColor);
            painter->fillRect(QRectF(0, sColorBarHeight, mWidth, mHeight - sColorBarHeight), GraphicsQssAdapter::instance()->nodeBackgroundColor());
            //            QRectF iconRect(sIconPadding,sIconPadding,sIconSize.width(),sIconSize.height());
            //            painter->fillRect(iconRect,Qt::black);

            // draw center text
            sPen.setColor(penColor(option->state, sTextColor));
            painter->setPen(sPen);

            for (int iline = 0; iline < 3; iline++)
            {
                if (mNodeText[iline].isEmpty())
                    continue;
                painter->setFont(sTextFont[iline]);
                painter->drawText(mTextPosition[iline], mNodeText[iline]);
            }

            bool moduleHasFocus = gSelectionRelay->focusType() == SelectionRelay::ItemType::Module && gSelectionRelay->focusId() == mId;
            int subFocusIndex   = static_cast<int>(gSelectionRelay->subfocusIndex());

            painter->setFont(sPinFont);
            QPointF text_pos(sPinOuterHorizontalSpacing, sColorBarHeight + sPinUpperVerticalSpacing + sPinFontAscent + sBaseline);

            for (int i = 0; i < mInputPins.size(); ++i)
            {
                if (moduleHasFocus)
                    if (gSelectionRelay->subfocus() == SelectionRelay::Subfocus::Left && i == subFocusIndex)
                        sPen.setColor(selectionColor());
                    else
                        sPen.setColor(sTextColor);
                else
                    sPen.setColor(penColor(option->state, sTextColor));
                painter->setPen(sPen);
                painter->drawText(text_pos, mInputPins.at(i).name);
                text_pos.setY(text_pos.y() + sPinFontHeight + sPinInnerVerticalSpacing);
            }

            for (int i = 0; i < mOutputPins.size(); ++i)
            {
                if (moduleHasFocus)
                    if (gSelectionRelay->subfocus() == SelectionRelay::Subfocus::Right && i == subFocusIndex)
                        sPen.setColor(selectionColor());
                    else
                        sPen.setColor(sTextColor);
                else
                    sPen.setColor(penColor(option->state, sTextColor));
                painter->setPen(sPen);
                painter->drawText(mOutputPinPositions.at(i), mOutputPins.at(i).name);
            }

            if (sLod < graph_widget_constants::sGateMaxLod)
            {
                QColor fade = mColor;
                fade.setAlphaF(sAlpha);
                painter->fillRect(QRectF(0, sColorBarHeight, mWidth, mHeight - sColorBarHeight), fade);
            }

            if (option->state & QStyle::State_Selected)
            {
                sPen.setColor(selectionColor());
                sPen.setCosmetic(true);
                painter->setPen(sPen);
                sPen.setCosmetic(false);
                bool original_antialiasing_value = painter->renderHints().testFlag(QPainter::Antialiasing);
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setBrush(QBrush());
                painter->drawRect(boundingRect());
                //painter->drawRect(boundingRect().marginsAdded(QMarginsF(0.5, 0.5, 0.5, 0.5)));
                painter->setRenderHint(QPainter::Antialiasing, original_antialiasing_value);
            }
            else
            {
                QColor gcol = groupingColor();
                if (gcol.isValid())
                {
                    QPen grPen(gcol);
                    painter->drawRect(boundingRect());
                }
            }
        }
    }

    float StandardGraphicsModule::yEndpointDistance() const
    {
        return (sPinFontHeight + sPinInnerVerticalSpacing);
    }

    float StandardGraphicsModule::yTopPinDistance() const
    {
        return (sColorBarHeight + sPinUpperVerticalSpacing + sPinFontHeight / 2);
    }

    QPointF StandardGraphicsModule::endpointPositionByIndex(int index, bool isInput) const
    {
        qreal y = yTopPinDistance() + index * yEndpointDistance();
        return mapToScene(QPointF(isInput ? 0 : mWidth, y));
    }

    void StandardGraphicsModule::format(const bool& adjust_size_to_grid)
    {
        QFontMetricsF pin_fm(sPinFont);
        qreal max_pin_width = 0;

        for (const ModulePin& input_pin : mInputPins)
        {
            qreal width = pin_fm.width(input_pin.name);
            if (width > max_pin_width)
                max_pin_width = width;
        }

        for (const ModulePin& output_pin : mOutputPins)
        {
            qreal width = pin_fm.width(output_pin.name);
            if (width > max_pin_width)
                max_pin_width = width;
        }

        qreal total_input_pin_height = 0;

        if (!mInputPins.isEmpty())
            total_input_pin_height = mInputPins.size() * sPinFontHeight + (mInputPins.size() - 1) * sPinInnerVerticalSpacing + sPinUpperVerticalSpacing + sPinLowerVerticalSpacing;

        qreal total_output_pin_height = 0;

        if (!mOutputPins.isEmpty())
            total_output_pin_height =
                mOutputPins.size() * sPinFontHeight + (mOutputPins.size() - 1) * sPinInnerVerticalSpacing + sPinUpperVerticalSpacing + sPinLowerVerticalSpacing;

        qreal max_pin_height  = std::max(total_input_pin_height, total_output_pin_height);
        qreal min_body_height = sInnerNameTypeSpacing + 2 * sOuterNameTypeSpacing;
        for (int iline = 0; iline < 3; iline++)
        {
            min_body_height += sTextFontHeight[iline];
        }

        mWidth  = max_pin_width * 2 + sPinInnerHorizontalSpacing * 2 + sPinOuterHorizontalSpacing * 2 + mMaxTextWidth;
        mHeight = std::max(max_pin_height, min_body_height) + sColorBarHeight;

        if (adjust_size_to_grid)
        {
            int floored_width = static_cast<int>(mWidth);
            int quotient      = floored_width / graph_widget_constants::sGridSize;

            if (mWidth > quotient * graph_widget_constants::sGridSize)
                mWidth = (quotient + 1) * graph_widget_constants::sGridSize;

            int floored_height = static_cast<int>(mHeight);
            quotient           = floored_height / graph_widget_constants::sGridSize;

            if (mHeight > quotient * graph_widget_constants::sGridSize)
                mHeight = (quotient + 1) * graph_widget_constants::sGridSize;
        }

        qreal ytext = std::max(mHeight / 2 - sTextFontHeight[0] * 3 / 2 - sInnerNameTypeSpacing / 2, sColorBarHeight + sOuterNameTypeSpacing);

        initTextPosition(ytext, sInnerNameTypeSpacing);

        qreal y = sColorBarHeight + sPinUpperVerticalSpacing + sPinFontAscent + sBaseline;

        for (const ModulePin& output_pin : mOutputPins)
        {
            qreal x = mWidth - (pin_fm.size(0, output_pin.name).rwidth() + sPinOuterHorizontalSpacing);
            mOutputPinPositions.append(QPointF(x, y));
            y += (sPinFontHeight + sPinInnerVerticalSpacing);
        }
    }
}    // namespace hal
