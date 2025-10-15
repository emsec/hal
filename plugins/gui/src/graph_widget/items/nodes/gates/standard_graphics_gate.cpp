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

qreal StandardGraphicsGate::sInverterCircleSize = 32;

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
            QPen pathPen(mColor,mPathWidth);
            pathPen.setJoinStyle(Qt::MiterJoin);
            pathPen.setMiterLimit(20);
            painter->setPen(pathPen);
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
    float xText = pin.isInput ? pin.x : pin.x - pin.textWidth;

    QPen storeCurrentPen = painter->pen();
    QBrush storeCurrentBrush = painter->brush();

    float wbox = pin.textWidth > sPinFontHeight ? pin.textWidth : sPinFontHeight;
    float xbox = pin.isInput ? pin.x : pin.x - wbox;

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
                        painter->setBrush(pinBackground);
                        painter->setPen(QPen(pinBackground,0));
                        painter->drawRoundRect(xbox,pin.y-sPinFontAscent,wbox,sPinFontHeight,35,35);
                        pinTextColor = legibleColor(pinBackground);
                    }
                }
            }
            sPen.setColor(pinTextColor);
    }

    if (mShapeType != StandardShape)
    {
        painter->setPen(QPen(mColor,1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundRect(xbox,pin.y-sPinFontAscent,wbox,sPinFontHeight,35,35);
    }

    painter->setPen(sPen);
    painter->drawText(QPointF(xText, pin.y), pin.name);
    painter->setPen(storeCurrentPen);
    painter->setBrush(storeCurrentBrush);
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

    for (auto it = mInputPinStruct.begin(); it != mInputPinStruct.end(); ++it)
    {
        qreal width = pin_fm.size(0, it->name).rwidth();
        it->textWidth = width;
        if (width > mMaxInputPinWidth)
            mMaxInputPinWidth = width;
    }

    for (auto it = mOutputPinStruct.begin(); it != mOutputPinStruct.end(); ++it)
    {
        qreal width = pin_fm.size(0, it->name).rwidth();
        it->textWidth = width;
        if (width > mMaxOutputPinWidth)
            mMaxOutputPinWidth = width;
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


    mHeight = mShapeType == StandardShape
            ? mHeight = std::max(max_pin_height, min_body_height) + sColorBarHeight
            : 2 * yTopPinDistance() + (mInputPinStruct.size() - 1.) * yEndpointDistance();

    float h = mHeight - mPathWidth;
    switch (mShapeType)
    {
        case InverterShape:
            mWidth = 2 * (sPinFontHeight + sPinOuterHorizontalSpacing) + 0.75 * h + 0.5 * mPathWidth;
            setMaxTextWidth(20);
            break;
        case AndShape:
        case NandShape:
            mWidth = 2 * (sPinFontHeight + sPinOuterHorizontalSpacing) + 1.3 * h - 0.7 * mInputPinStruct.size() * yEndpointDistance() - mPathWidth;
            setMaxTextWidth(60);
            break;
        case OrShape:
        case NorShape:
            mWidth = 2 * (sPinFontHeight + sPinOuterHorizontalSpacing) + 1.6 * h - 0.7 * mInputPinStruct.size() * yEndpointDistance() - mPathWidth;
            setMaxTextWidth(60);
            break;
        case XorShape:
        case NxorShape:
            mWidth = 2 * (sPinFontHeight + sPinOuterHorizontalSpacing) + 1.6 * h - 0.7 * mInputPinStruct.size() * yEndpointDistance() + mPathWidth;
            setMaxTextWidth(60);
            break;
        default:
            mWidth = mMaxInputPinWidth + mMaxOutputPinWidth + sPinInnerHorizontalSpacing * 2 + sPinOuterHorizontalSpacing * 2 + mMaxTextWidth;
            break;
    }

    if (inverterCircleShape(mShapeType))
    {
        mWidth += sInverterCircleSize - 0.5 * mPathWidth;
    }

    if (adjust_size_to_grid && mShapeType == StandardShape)
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

    switch (mShapeType)
    {
        case InverterShape:
            initTextPosition(ytext0 - sTextFontHeight[0], sInnerNameTypeSpacing, sPinFontHeight + 2*sPinOuterHorizontalSpacing + mPathWidth);
            break;
        case AndShape:
        case NandShape:
            initTextPosition(ytext0, sInnerNameTypeSpacing, sPinFontHeight + 2*sPinOuterHorizontalSpacing + mPathWidth);
            break;
        case OrShape:
        case NorShape:
            initTextPosition(ytext0, sInnerNameTypeSpacing, sPinFontHeight + 2*sPinOuterHorizontalSpacing + 2*mPathWidth);
            break;
        case XorShape:
        case NxorShape:
            initTextPosition(ytext0, sInnerNameTypeSpacing, sPinFontHeight + 2*sPinOuterHorizontalSpacing + 4*mPathWidth);
            break;
        default:
            initTextPosition(ytext0, sInnerNameTypeSpacing);
            break;
    }

    qreal y = sColorBarHeight + sPinUpperVerticalSpacing + sPinFontAscent + sBaseline;


    int x0 = mPathWidth/2 + sPinFontHeight + sPinOuterHorizontalSpacing;
    int y0 = mPathWidth/2;
    int x1 = mWidth  - mPathWidth/2 - sPinFontHeight - sPinOuterHorizontalSpacing;
    int y1 = mHeight - mPathWidth/2;
    float diam = (y1-y0)*2;
    float x60deg = sqrt(3)/2*diam/2;
    double arcSpan = sin((diam/2-mPathWidth)/diam)*180./M_PI;
    float invDiam = sInverterCircleSize - mPathWidth;

    mPath.clear();
    if (inverterCircleShape(mShapeType))
    {
        mPath.arcMoveTo(x1-invDiam,(mHeight-invDiam)/2,invDiam,invDiam,0);
        mPath.arcTo(x1-invDiam,(mHeight-invDiam)/2,invDiam,invDiam,0,360);
        mPath.closeSubpath();
        x1 -= invDiam + mPathWidth/2;
    }

    switch (mShapeType)
    {
        case InverterShape:
        {
            float top = (y1-y0)/2.;
            mPath.moveTo(x0,y0);
            mPath.lineTo(x1,y0+top);
            mPath.lineTo(x0,y1);
            mPath.closeSubpath();
            break;
        }

        case AndShape:
        case NandShape:
        {
            diam = y1-y0;
            mPath.moveTo(x0,y0);
           // mPath.lineTo(x1-diam/2,y0);
            mPath.arcTo(x1-diam,y0,diam,diam,90,-180);
            mPath.lineTo(x0,y1);
            mPath.closeSubpath();
            break;
        }

        case XorShape:
        case NxorShape:
            mPath.arcMoveTo(x0-x60deg-diam/2,y0-diam/4,diam,diam,30);
            mPath.arcTo(x0-x60deg-diam/2,y0-diam/4,diam,diam,30,-60);
            x0 += 2*mPathWidth;
            // fallthrough -> OrShape

        case OrShape:
        case NorShape:
        {
            mPath.moveTo(x0,y0);
            mPath.lineTo(x1-x60deg,y0);
            mPath.arcTo(x1-x60deg-diam/2,y0,diam,diam,90,-60);
            mPath.arcTo(x1-x60deg-diam/2,y1-diam,diam,diam,-30,-60);
            mPath.lineTo(x0,y1);
            mPath.lineTo(x0+mPathWidth/4,y1-mPathWidth/2); // line in arc direction for pointy corner
            mPath.arcTo(x0-x60deg-diam/2,y0-diam/4,diam,diam,-arcSpan,2*arcSpan);
            mPath.closeSubpath();
            break;
        }

        default:
            break;
    }

    setPinPosition();
}

bool StandardGraphicsGate::inverterCircleShape(ShapeType shapeType)
{
    switch (shapeType)
    {
        case InverterShape:
        case NandShape:
        case NorShape:
        case NxorShape:
            return true;
        default:
            break;
    }

    return false;
}

void StandardGraphicsGate::setPinPosition()
{
    int yFirstTextline = sColorBarHeight + sPinUpperVerticalSpacing + sPinFontAscent + sBaseline;

    for (auto it = mInputPinStruct.begin(); it != mInputPinStruct.end(); ++it)
    {
        if (mShapeType == OrShape || mShapeType == XorShape || mShapeType == NorShape || mShapeType == NxorShape )
        {
            float dx = mHeight*(1.-sqrt(3)/2.);
            it->x = sPinOuterHorizontalSpacing + dx;
            it->connectors.append(QLineF(0,0,dx,0));
        }
        else
            it->x = sPinOuterHorizontalSpacing;
        it->y = yFirstTextline + it->index * (sPinFontHeight + sPinInnerVerticalSpacing);
    }

    if (mShapeType != StandardShape && mOutputPinStruct.size() == 1)
    {
        auto it = mOutputPinStruct.begin();
        it->x = mWidth - sPinOuterHorizontalSpacing;
        it->y = (mHeight - sPinFontHeight) / 2 + sPinFontAscent;
        float yl = mHeight/2 - yTopPinDistance();
        it->connectors.append(QLineF(-sPinOuterHorizontalSpacing, yl, 0, yl));
        if(yl != 0) it->connectors.append(QLineF(0, 0, 0, yl));
    }
    else
    {
        for (auto it = mOutputPinStruct.begin(); it != mOutputPinStruct.end(); ++it)
        {
            it->x = mWidth - sPinOuterHorizontalSpacing;
            it->y = yFirstTextline + it->index * (sPinFontHeight + sPinInnerVerticalSpacing);
        }
    }
}
}
