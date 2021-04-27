#include "gui/style/style_manager.h"

#include "hal_core/utilities/log.h"

#include "gui/graph_widget/graphics_qss_adapter.h"
#include "gui/code_editor/syntax_highlighter/python_qss_adapter.h"
#include "gui/code_editor/syntax_highlighter/vhdl_qss_adapter.h"
#include "gui/style/shared_properties_qss_adapter.h"
#include "gui/gui_globals.h"

#include <QApplication>
#include <QFile>

namespace hal
{
    StyleManager::StyleManager(QObject* parent)
        : QObject(parent), mGraphicsQssAdapter(new GraphicsQssAdapter()), mSharedProperties(new SharedPropertiesQssAdapter()),
          mPythonSyntaxHighlighter(new PythonQssAdapter()), mVhdlSyntaxHighlighter(new VhdlQssAdapter())
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

    const VhdlQssAdapter* StyleManager::VhdlSyntaxHighlighter() const
    {
        return mVhdlSyntaxHighlighter;
    }

    void StyleManager::repolish()
    {
        mGraphicsQssAdapter->repolish();
    }
}
