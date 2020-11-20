#include "gui/graphics_effects/graphics_effect_factory.h"

namespace hal
{
    namespace GraphicsEffectFactory
    {
        QGraphicsEffect* getEffect()
        {
            QGraphicsBlurEffect* effect = new QGraphicsBlurEffect(nullptr);
            effect->setBlurHints(QGraphicsBlurEffect::QualityHint);
            effect->setBlurRadius(6);
            return effect;
        }
    }
}
