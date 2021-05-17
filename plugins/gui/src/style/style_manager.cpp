#include "gui/style/style_manager.h"

#include "gui/code_editor/syntax_highlighter/python_qss_adapter.h"
#include "gui/graph_widget/graphics_qss_adapter.h"
#include "gui/gui_globals.h"
#include "gui/style/shared_properties_qss_adapter.h"
#include "hal_core/utilities/log.h"

#include <QApplication>
#include <QFile>

namespace hal
{
    StyleManager::StyleManager(QObject* parent)
        : QObject(parent), mGraphicsQssAdapter(new GraphicsQssAdapter()), mSharedProperties(new SharedPropertiesQssAdapter()), mPythonSyntaxHighlighter(new PythonQssAdapter())
    {
    }

    StyleManager* StyleManager::get_instance()
    {
        static StyleManager instance;
        return &instance;
    }

    const SharedPropertiesQssAdapter* StyleManager::sharedProperties() const
    {
        return mSharedProperties;
    }

    const PythonQssAdapter* StyleManager::PythonSyntaxHighlighter() const
    {
        return mPythonSyntaxHighlighter;
    }

    void StyleManager::repolish()
    {
        mGraphicsQssAdapter->repolish();
    }
}    // namespace hal
