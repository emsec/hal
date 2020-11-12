#include "gui/graph_widget/graphics_qss_adapter.h"

#include <QStyle>

namespace hal
{
    GraphicsQssAdapter::GraphicsQssAdapter(QWidget* parent) : QWidget(parent),
        mGateBaseColor(255, 200, 0),
        mNetBaseColor(255, 200, 0),
        mGateSelectionColor(255, 200, 0),
        mNetSelectionColor(255, 200, 0),
        mGridBaseLineColor(255, 200, 0),
        mGridClusterLineColor(255, 200, 0),
        mGridBaseDotColor(255, 200, 0),
        mGridClusterDotColor(255, 200, 0)
    {
        ensurePolished();
    }

    void GraphicsQssAdapter::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        // SET STATICS OR ADRESS ADAPTER MEMBERS DIRECTLY
    }

    QColor GraphicsQssAdapter::gateBaseColor() const
    {
        return mGateBaseColor;
    }

    QColor GraphicsQssAdapter::netBaseColor() const
    {
        return mNetBaseColor;
    }

    QColor GraphicsQssAdapter::gateSelectionColor() const
    {
        return mGateSelectionColor;
    }

    QColor GraphicsQssAdapter::netSelectionColor() const
    {
        return mNetSelectionColor;
    }

    QFont GraphicsQssAdapter::gateNameFont() const
    {
        return mGateNameFont;
    }

    QFont GraphicsQssAdapter::gateTypeFont() const
    {
        return mGateTypeFont;
    }

    QFont GraphicsQssAdapter::gatePinFont() const
    {
        return mGatePinFont;
    }

    QFont GraphicsQssAdapter::netFont() const
    {
        return mNetFont;
    }

    QColor GraphicsQssAdapter::gridBaseLineColor() const
    {
        return mGridBaseLineColor;
    }

    QColor GraphicsQssAdapter::gridClusterLineColor() const
    {
        return mGridClusterLineColor;
    }

    QColor GraphicsQssAdapter::gridBaseDotColor() const
    {
        return mGridBaseDotColor;
    }

    QColor GraphicsQssAdapter::gridClusterDotColor() const
    {
        return mGridClusterDotColor;
    }

    void GraphicsQssAdapter::setGateBaseColor(const QColor& color)
    {
        mGateBaseColor = color;
    }

    void GraphicsQssAdapter::setNetBaseColor(const QColor& color)
    {
        mNetBaseColor = color;
    }

    void GraphicsQssAdapter::setGateSelectionColor(const QColor& color)
    {
        mGateSelectionColor = color;
    }

    void GraphicsQssAdapter::setNetSelectionColor(const QColor& color)
    {
        mNetSelectionColor = color;
    }

    void GraphicsQssAdapter::setGateNameFont(const QFont& font)
    {
        mGateNameFont = font;
    }

    void GraphicsQssAdapter::setGateTypeFont(const QFont& font)
    {
        mGateTypeFont = font;
    }

    void GraphicsQssAdapter::setGatePinFont(const QFont& font)
    {
        mGatePinFont = font;
    }

    void GraphicsQssAdapter::setNetFont(const QFont& font)
    {
        mNetFont = font;
    }

    void GraphicsQssAdapter::setGridBaseLineColor(const QColor& color)
    {
        mGridBaseLineColor = color;
    }

    void GraphicsQssAdapter::setGridClusterLineColor(const QColor& color)
    {
        mGridClusterLineColor = color;
    }

    void GraphicsQssAdapter::setGridBaseDotColor(const QColor& color)
    {
        mGridBaseDotColor = color;
    }

    void GraphicsQssAdapter::setGridClusterDotColor(const QColor& color)
    {
        mGridClusterDotColor = color;
    }
}
