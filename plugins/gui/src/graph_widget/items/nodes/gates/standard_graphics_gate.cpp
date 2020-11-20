#include "gui/graph_widget/items/nodes/gates/standard_graphics_gate.h"

#include "hal_core/netlist/gate.h"

#include "gui/graph_widget/graph_widget_constants.h"
#include "gui/gui_globals.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPen>
#include <QStyle>
#include <QStyleOptionGraphicsItem>

namespace hal{
static const qreal sBaseline = 1;

qreal StandardGraphicsGate::sAlpha;

QPen StandardGraphicsGate::sPen;

QColor StandardGraphicsGate::sTextColor;

QFont StandardGraphicsGate::sTextFont[2];
QFont StandardGraphicsGate::sPinFont;

qreal StandardGraphicsGate::sTextFontHeight[2];

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
QPixmap* StandardGraphicsGate::sIconInstance = nullptr;

const QPixmap& StandardGraphicsGate::iconPixmap()
{
    if (!sIconInstance) sIconInstance
            = new QPixmap(QPixmap::fromImage(QImage(":/icons/sel_gate").scaled(sIconSize)));
    return *sIconInstance;
}


void StandardGraphicsGate::loadSettings()
{
    sPen.setCosmetic(true);
    sPen.setJoinStyle(Qt::MiterJoin);

    sTextColor = QColor(160, 160, 160);

    QFont font = QFont("Iosevka");
    font.setPixelSize(graph_widget_constants::sFontSize);

    for (int iline=0; iline<2; iline++)
    {
        sTextFont[iline] = font;
        QFontMetricsF fmf(font);
        sTextFontHeight[iline] = fmf.height();
    }

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

    if (sLod < graph_widget_constants::sGateMinLod)
    {
        painter->fillRect(QRectF(0, 0, mWidth, mHeight), penColor(option->state));
        return;
    }
    else
    {
        painter->fillRect(QRectF(0, 0, mWidth, sColorBarHeight), mColor);
        painter->fillRect(QRectF(0, sColorBarHeight, mWidth, mHeight - sColorBarHeight), QColor(0, 0, 0, 200));
//        QRectF iconRect(sIconPadding,sIconPadding,sIconSize.width(),sIconSize.height());
//        painter->fillRect(iconRect,Qt::black);
//        painter->drawPixmap(QPoint(sIconPadding,sIconPadding), iconPixmap());

        sPen.setColor(penColor(option->state,sTextColor));
        painter->setPen(sPen);

        for (int iline=0; iline<2; iline++)
        {
            painter->setFont(sTextFont[iline]);
            painter->drawText(mTextPosition[iline], mNodeText[iline]);
        }

        bool gateHasFocus =
                gSelectionRelay->mFocusType == SelectionRelay::ItemType::Gate
                && gSelectionRelay->mFocusId == mId;
        int subFocusIndex = static_cast<int>(gSelectionRelay->mSubfocusIndex);

        painter->setFont(sPinFont);
        sPen.setColor(sTextColor);
        painter->setPen(sPen);

        QPointF text_pos(sPinOuterHorizontalSpacing, sColorBarHeight + sPinUpperVerticalSpacing + sPinFontAscent + sBaseline);

        for (int i = 0; i < mInputPins.size(); ++i)
        {
            if (gateHasFocus)
                if (gSelectionRelay->mSubfocus == SelectionRelay::Subfocus::Left
                        && i == subFocusIndex)
                    sPen.setColor(selectionColor());
                else
                    sPen.setColor(sTextColor);
            else
                sPen.setColor(penColor(option->state,sTextColor));
            painter->setPen(sPen);
            painter->drawText(text_pos, mInputPins.at(i));
            text_pos.setY(text_pos.y() + sPinFontHeight + sPinInnerVerticalSpacing);
        }

        for (int i = 0; i < mOutputPins.size(); ++i)
        {
            if (gateHasFocus)
                if (gSelectionRelay->mSubfocus == SelectionRelay::Subfocus::Right
                        && i == subFocusIndex)
                    sPen.setColor(selectionColor());
                else
                    sPen.setColor(sTextColor);
            else
               sPen.setColor(penColor(option->state,sTextColor));
            painter->setPen(sPen);
            painter->drawText(mOutputPinPositions.at(i), mOutputPins.at(i));
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
                painter->setPen(grPen);
                painter->drawRect(boundingRect());
            }
        }
    }
}

QPointF StandardGraphicsGate::getInputScenePosition(const u32 mNetId, const QString& pin_type) const
{
    Q_UNUSED(mNetId)

    int index = mInputPins.indexOf(pin_type);
    assert(index != -1);

    return endpointPositionByIndex(index,true);
}

QPointF StandardGraphicsGate::getOutputScenePosition(const u32 mNetId, const QString& pin_type) const
{
    Q_UNUSED(mNetId)

    int index = mOutputPins.indexOf(pin_type);
    assert(index != -1);

    return endpointPositionByIndex(index,false);
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
    qreal textWidth[2];
    for (int iline=0; iline<2; iline++)
    {
        QFontMetricsF fmf(sTextFont[iline]);
        textWidth[iline] = fmf.width(mNodeText[iline]);
    }

    QFontMetricsF pin_fm(sPinFont);
    qreal max_pin_width = 0;

    for (const QString& input_pin : mInputPins)
    {
        qreal width = pin_fm.width(input_pin);
        if (width > max_pin_width)
            max_pin_width = width;
    }

    for (const QString& output_pin : mOutputPins)
    {
        qreal width = pin_fm.width(output_pin);
        if (width > max_pin_width)
            max_pin_width = width;
    }

    qreal total_input_pin_height = 0;

    if (!mInputPins.isEmpty())
        total_input_pin_height = mInputPins.size() * sPinFontHeight +
                                (mInputPins.size() - 1) * sPinInnerVerticalSpacing +
                                 sPinUpperVerticalSpacing + sPinLowerVerticalSpacing;

    qreal total_output_pin_height = 0;

    if (!mOutputPins.isEmpty())
        total_output_pin_height = mOutputPins.size() * sPinFontHeight +
                                 (mOutputPins.size() - 1) * sPinInnerVerticalSpacing +
                                  sPinUpperVerticalSpacing + sPinLowerVerticalSpacing;

    qreal max_pin_height = std::max(total_input_pin_height, total_output_pin_height);
    qreal min_body_height = sInnerNameTypeSpacing + 2 * sOuterNameTypeSpacing;

    qreal maxTextWidth = 0;
    for (int iline=0; iline<2; iline++)
    {
        min_body_height += sTextFontHeight[iline];
        if (maxTextWidth  < textWidth[iline]) maxTextWidth = textWidth[iline];
    }


    mWidth = max_pin_width * 2 + sPinInnerHorizontalSpacing * 2 + sPinOuterHorizontalSpacing * 2 + maxTextWidth;
    mHeight = std::max(max_pin_height, min_body_height) + sColorBarHeight;

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

    mTextPosition[0].setX(mWidth / 2 - textWidth[0] / 2);
    mTextPosition[0].setY(std::max(mHeight / 2 - sTextFontHeight[0] / 2 - sInnerNameTypeSpacing / 2,
                          sColorBarHeight + sOuterNameTypeSpacing + sTextFontHeight[0]));

    mTextPosition[1].setX(mWidth / 2 - textWidth[1] / 2);
    mTextPosition[1].setY(mTextPosition[0].y() + sTextFontHeight[1] + sInnerNameTypeSpacing / 2);

    qreal y = sColorBarHeight + sPinUpperVerticalSpacing + sPinFontAscent + sBaseline;

    for (const QString& output_pin : mOutputPins)
    {
        qreal x = mWidth - (pin_fm.size(0, output_pin).rwidth() + sPinOuterHorizontalSpacing);
        mOutputPinPositions.append(QPointF(x, y));
        y += (sPinFontHeight + sPinInnerVerticalSpacing);
    }
}
}
