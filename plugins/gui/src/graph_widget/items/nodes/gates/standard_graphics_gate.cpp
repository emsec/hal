#include "gui/graph_widget/items/nodes/gates/standard_graphics_gate.h"

#include "hal_core/netlist/gate.h"

#include "gui/content_manager/content_manager.h"
#include "gui/graph_widget/graph_widget_constants.h"
#include "gui/graph_widget/graphics_qss_adapter.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"
#include "gui/settings/settings_items/settings_item_checkbox.h"
#include "gui/graph_widget/graph_context_manager.h"
#include "gui/gui_globals.h"

#include <QPainter>
#include <QPen>
#include <QStyle>
#include <QStyleOptionGraphicsItem>

namespace hal{
static const qreal sBaseline = 1;

qreal StandardGraphicsGate::sAlpha;

QPen StandardGraphicsGate::sPen;

QFont StandardGraphicsGate::sPinFont;

qreal StandardGraphicsGate::sColorBarHeight = 30;

qreal StandardGraphicsGate::sPinInnerHorizontalSpacing = 12;
qreal StandardGraphicsGate::sPinOuterHorizontalSpacing = 2.4;

qreal StandardGraphicsGate::sPinInnerVerticalSpacing = 1.2;
qreal StandardGraphicsGate::sPinOuterVerticalSpacing = 0.6;
qreal StandardGraphicsGate::sPinUpperVerticalSpacing = 2;
qreal StandardGraphicsGate::sPinLowerVerticalSpacing = 1.8;

qreal StandardGraphicsGate::sPinFontHeight;
qreal StandardGraphicsGate::sPinFontAscent;
qreal StandardGraphicsGate::sPinFontDescent;
qreal StandardGraphicsGate::sPinFontBaseline;

qreal StandardGraphicsGate::sInnerNameTypeSpacing = 1.2;
qreal StandardGraphicsGate::sOuterNameTypeSpacing = 3;

qreal StandardGraphicsGate::sFirstPinY;
qreal StandardGraphicsGate::sPinYStride;

const int StandardGraphicsGate::sIconPadding = 3;
const QSize StandardGraphicsGate::sIconSize(sColorBarHeight - 2 * sIconPadding,
                                              sColorBarHeight - 2 * sIconPadding);
void StandardGraphicsGate::loadSettings()
{
    sPen.setCosmetic(true);
    sPen.setJoinStyle(Qt::MiterJoin);

    sTextColor = GraphicsQssAdapter::instance()->nodeTextColor();

    QFont font = QFont("Iosevka");
    font.setPixelSize(graph_widget_constants::sFontSize);

    sPinFont = font;
    QFontMetricsF pin_fm(sPinFont);
    sPinFontHeight = pin_fm.height();
    sPinFontAscent = pin_fm.ascent();
    sPinFontDescent = pin_fm.descent();
    sPinFontBaseline = 1;

    sFirstPinY = sColorBarHeight + sPinUpperVerticalSpacing + sPinFontHeight;
    sPinYStride = sPinFontHeight + sPinInnerVerticalSpacing;
}

void StandardGraphicsGate::updateAlpha()
{
    if (sLod <= graph_widget_constants::sGateMaxLod)
        sAlpha = 1 - (sLod - graph_widget_constants::sGateMinLod) / (graph_widget_constants::sGateMaxLod - graph_widget_constants::sGateMinLod);
    else
        sAlpha = 0;
}

StandardGraphicsGate::StandardGraphicsGate(Gate* g, const bool adjust_size_to_grid) : GraphicsGate(g)
{
    format(adjust_size_to_grid);
}

void StandardGraphicsGate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);

//  --- rect shape on zoom out ---
    if (sLod < graph_widget_constants::sGateMinLod)
    {
        painter->fillRect(QRectF(0, 0, mWidth, mHeight), penColor(option->state));
        return;
    }
    else
    {

//      --- filled color bar and background rect ---

        if (mShapeType == StandardShape)
        {
            painter->fillRect(QRectF(0, 0, mWidth, sColorBarHeight), mColor);
            painter->fillRect(QRectF(0, sColorBarHeight, mWidth, mHeight - sColorBarHeight), GraphicsQssAdapter::instance()->nodeBackgroundColor());
        }
        else
        {
            painter->setPen(QPen(mColor,12));
            painter->setBrush(GraphicsQssAdapter::instance()->nodeBackgroundColor());
            painter->drawPath(mPath);
        }

//        --- added icon to box ---
//        QRectF iconRect(sIconPadding,sIconPadding,sIconSize.width(),sIconSize.height());
//        painter->fillRect(iconRect,Qt::black);

//      --- draw text ---
        sPen.setColor(penColor(option->state,sTextColor));
        painter->setPen(sPen);

        for (int iline=0; iline<2; iline++)
        {
            painter->setFont(sTextFont[iline]);
            painter->drawText(mTextPosition[iline], mNodeText[iline]);
        }

//      --- prepare focus variable ---
        bool gateHasFocus =
                gSelectionRelay->focusType() == SelectionRelay::ItemType::Gate
                && gSelectionRelay->focusId() == mId;
        int subFocusIndex = static_cast<int>(gSelectionRelay->subfocusIndex());

//      --- pin text color ---
        painter->setFont(sPinFont);
        sPen.setColor(sTextColor);
        painter->setPen(sPen);

        int yFirstTextline = sColorBarHeight + sPinUpperVerticalSpacing + sPinFontAscent + sBaseline;

        for (const struct GraphicsGatePin& inpPin : mInputPinStruct)
        {
            FocusType focusType = NoFocus;
            if (gateHasFocus)
            {
                if (gSelectionRelay->subfocus() == SelectionRelay::Subfocus::Left && inpPin.index == subFocusIndex)
                    focusType = PinFocus;
                else
                    focusType = GateFocus;
            }
            paintPin(painter, option->state, inpPin, focusType);
        }

        for (const struct GraphicsGatePin& outPin : mOutputPinStruct)
        {
            FocusType focusType = NoFocus;
            if (gateHasFocus)
            {
                if (gSelectionRelay->subfocus() == SelectionRelay::Subfocus::Right && outPin.index == subFocusIndex)
                    focusType = PinFocus;
                else
                    focusType = GateFocus;
            }
            paintPin(painter, option->state, outPin, focusType);
        }

//      --- fade on zoom out ---
        if (sLod < graph_widget_constants::sGateMaxLod)
        {
            QColor fade = mColor;
            fade.setAlphaF(sAlpha);
            painter->fillRect(QRectF(0, sColorBarHeight, mWidth, mHeight - sColorBarHeight), fade);
        }

//      --- box around ---
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
                painter->setPen(grPen);
                painter->drawRect(boundingRect());
            }
        }
    }
}

void StandardGraphicsGate::paintPin(QPainter* painter, QStyle::State state, const GraphicsGatePin& pin, FocusType focusType)
{
    int yFirstTextline = sColorBarHeight + sPinUpperVerticalSpacing + sPinFontAscent + sBaseline;

    float yText = yFirstTextline + pin.index * (sPinFontHeight + sPinInnerVerticalSpacing);
    float xText = pin.isInput ? sPinOuterHorizontalSpacing : mWidth - sPinOuterHorizontalSpacing - pin.textWidth;

    QPen storeCurrentPen = painter->pen();

    switch (focusType) {
        case PinFocus:
            sPen.setColor(selectionColor());
            break;
        case GateFocus:
            sPen.setColor(sTextColor);
            break;
        default:
            QColor pinTextColor = penColor(state,sTextColor);
            if (pin.netId)
            {
                if (gGraphContextManager->sSettingNetGroupingToPins->value().toBool())
                {
                    QColor pinBackground = gContentManager->getGroupingManagerWidget()->getModel()->colorForItem(ItemType::Net, pin.netId);
                    if (pinBackground.isValid())
                    {
                        QBrush storeCurrentBrush = painter->brush();
                        painter->setBrush(pinBackground);
                        painter->setPen(QPen(pinBackground,0));
                        float wbox = pin.textWidth > sPinFontHeight ? pin.textWidth : sPinFontHeight;
                        float xbox = pin.isInput ? sPinOuterHorizontalSpacing : mWidth - sPinOuterHorizontalSpacing - wbox;
                        painter->drawRoundRect(xbox,yText-sPinFontAscent,wbox,sPinFontHeight,35,35);
                        painter->setBrush(storeCurrentBrush);
                        pinTextColor = legibleColor(pinBackground);
                    }
                }
            }
            sPen.setColor(pinTextColor);
    }
    painter->setPen(sPen);
    painter->drawText(QPointF(xText, yText), pin.name);
    painter->setPen(storeCurrentPen);
}

QColor StandardGraphicsGate::legibleColor(const QColor& bgColor)
{
    // brightness of color according to YUV color scheme
    const static float kr = 0.229;
    const static float kb = 0.114;
    float y = kr*bgColor.red() + (1-kr-kb)*bgColor.green() + kb*bgColor.blue();

    if (y > 127.5) return QColor(Qt::black);
    return QColor(Qt::white);
}

float StandardGraphicsGate::yEndpointDistance() const
{
    return (sPinFontHeight + sPinInnerVerticalSpacing);
}

float StandardGraphicsGate::yTopPinDistance() const
{
    return (sColorBarHeight + sPinUpperVerticalSpacing + sPinFontHeight / 2);
}

QPointF StandardGraphicsGate::endpointPositionByIndex(int index, bool isInput) const
{
    qreal y = yTopPinDistance() + index * yEndpointDistance();
    return mapToScene(QPointF(isInput ? 0 : mWidth, y));
}

void StandardGraphicsGate::format(const bool& adjust_size_to_grid)
{
    QFontMetricsF pin_fm(sPinFont);
    qreal max_pin_width = 0;

    for (auto it = mInputPinStruct.begin(); it != mInputPinStruct.end(); ++it)
    {
        qreal width = pin_fm.size(0, it->name).rwidth();
        it->textWidth = width;
        if (width > max_pin_width)
            max_pin_width = width;
    }

    for (auto it = mOutputPinStruct.begin(); it != mOutputPinStruct.end(); ++it)
    {
        qreal width = pin_fm.size(0, it->name).rwidth();
        it->textWidth = width;
        if (width > max_pin_width)
            max_pin_width = width;
    }

    qreal total_input_pin_height = 0;    
    int ni = mInputPinStruct.size();
    if (ni)
        total_input_pin_height = ni * sPinFontHeight +
                                (ni - 1) * sPinInnerVerticalSpacing +
                                 sPinUpperVerticalSpacing + sPinLowerVerticalSpacing;

    qreal total_output_pin_height = 0;
    int no = mOutputPinStruct.size();
    if (no)
        total_output_pin_height = no * sPinFontHeight +
                                 (no - 1) * sPinInnerVerticalSpacing +
                                  sPinUpperVerticalSpacing + sPinLowerVerticalSpacing;

    qreal max_pin_height = std::max(total_input_pin_height, total_output_pin_height);
    qreal min_body_height = sInnerNameTypeSpacing + 2 * sOuterNameTypeSpacing;

    for (int iline=0; iline<2; iline++)
    {
        min_body_height += sTextFontHeight[iline];
    }


    mWidth = max_pin_width * 2 + sPinInnerHorizontalSpacing * 2 + sPinOuterHorizontalSpacing * 2 + mMaxTextWidth;
    mHeight = std::max(max_pin_height, min_body_height) + sColorBarHeight;
    if (mShapeType == StandardShape)
        mHeight = std::max(max_pin_height, min_body_height) + sColorBarHeight;
    else
        mHeight = max_pin_height + 2*sColorBarHeight;

    if (adjust_size_to_grid)
    {
        int floored_width = static_cast<int>(mWidth);
        int quotient = floored_width / graph_widget_constants::sGridSize;

        if (mWidth > quotient * graph_widget_constants::sGridSize)
            mWidth = (quotient + 1) * graph_widget_constants::sGridSize;

        int floored_height = static_cast<int>(mHeight);
        quotient = floored_height / graph_widget_constants::sGridSize;

        if (mHeight > quotient * graph_widget_constants::sGridSize)
            mHeight = (quotient + 1) * graph_widget_constants::sGridSize;
    }

    // reproduce formatting, sTextFontHeight[0] will be added befor placing line
    qreal ytext0 = std::max(mHeight / 2 - sTextFontHeight[0] * 3 / 2 - sInnerNameTypeSpacing / 2,
                          sColorBarHeight + sOuterNameTypeSpacing);
    initTextPosition(ytext0, sInnerNameTypeSpacing);

    qreal y = sColorBarHeight + sPinUpperVerticalSpacing + sPinFontAscent + sBaseline;


    int mLineWidth = 12;
    int x0 = mLineWidth/2;
    int y0 = mLineWidth/2;
    int x1 = mWidth  - mLineWidth/2;
    int y1 = mHeight - mLineWidth/2;

    mPath.clear();
    switch (mShapeType)
    {
        case InverterShape:
        {
            float top = (y1-y0)/2.;
            float diam = top/3;
            mPath.moveTo(x0,y0);
            mPath.lineTo(x0+top+diam,y0+top);
            mPath.arcTo(x0+top+diam,y0+top-diam/2,diam,diam,-180,360);
            mPath.lineTo(x0,y1);
            mPath.closeSubpath();
            break;
        }

        case AndShape:
        {
            int diam = y1-y0;
            mPath.moveTo(x0,y0);
            mPath.lineTo(x1-diam/2,y0);
            mPath.arcTo(x1-diam,y0,diam,diam,90,-180);
            mPath.lineTo(x0,y1);
            mPath.closeSubpath();
            break;
        }

        case OrShape:
        {
            float diam = (y1-y0)*2;
            float x60deg = sqrt(3)/2*diam/2;
            mPath.moveTo(x0,y0);
            mPath.lineTo(x1-x60deg,y0);
            mPath.arcTo(x1-x60deg-diam/2,y0,diam,diam,90,-60);
            mPath.arcTo(x1-x60deg-diam/2,y1-diam,diam,diam,-30,-60);
            mPath.lineTo(x0,y1);
            mPath.arcTo(x0-x60deg-diam/2,y0-diam/4,diam,diam,-30,60);
            break;
        }

        default:
            break;
    }

}
}
