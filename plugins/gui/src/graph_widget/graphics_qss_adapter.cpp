#include "gui/graph_widget/graphics_qss_adapter.h"

#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/shaders/module_shader.h"
#include "gui/graph_widget/items/nodes/gates/standard_graphics_gate.h"
#include "gui/graph_widget/items/nodes/modules/standard_graphics_module.h"
#include <QStyle>

namespace hal
{
    GraphicsQssAdapter* GraphicsQssAdapter::inst = nullptr;

    GraphicsQssAdapter::GraphicsQssAdapter(QWidget* parent) : QWidget(parent),
        mGateBaseColor(255, 200, 0),
        mNetBaseColor(255, 200, 0),
        mGateSelectionColor(255, 200, 0),
        mNetSelectionColor(255, 200, 0),
        mGridBaseLineColor(30, 30, 30),
        mGridClusterLineColor(15, 15, 15),
        mGridBaseDotColor(25, 25, 25),
        mGridClusterDotColor(170, 160, 125),
        mNodeBackgroundColor(0,0,0,200),
        mNodeTextColor(160,160,160)
    {
        ensurePolished();
        repolish();
    }

    GraphicsQssAdapter* GraphicsQssAdapter::instance()
    {
        if (!inst) inst = new GraphicsQssAdapter;
        return inst;
    }

    void GraphicsQssAdapter::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        ModuleShader::debugSetNetColor(mNetBaseColor);
        StandardGraphicsGate::sTextColor = mNodeTextColor;
        StandardGraphicsModule::sTextColor = mNodeTextColor;
    }

    void GraphicsQssAdapter::setGridAlpha(int alpha)
    {
        mGridBaseLineColor.setAlpha(alpha);
        mGridClusterLineColor.setAlpha(alpha);

        mGridBaseDotColor.setAlpha(alpha);
        mGridClusterDotColor.setAlpha(alpha);
    }

    void GraphicsQssAdapter::setGridAlphaF(qreal alpha)
    {
        mGridBaseLineColor.setAlphaF(alpha);
        mGridClusterLineColor.setAlphaF(alpha);

        mGridBaseDotColor.setAlphaF(alpha);
        mGridClusterDotColor.setAlphaF(alpha);
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

    QColor GraphicsQssAdapter::nodeBackgroundColor() const
    {
        return mNodeBackgroundColor;
    }

    QColor GraphicsQssAdapter::nodeTextColor() const
    {
        return mNodeTextColor;
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

    void GraphicsQssAdapter::setNodeBackgroundColor(const QColor &color)
    {
        mNodeBackgroundColor = color;
    }

    void GraphicsQssAdapter::setNodeTextColor(const QColor &color)
    {
        mNodeTextColor = color;
    }
}
