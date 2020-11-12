#include "gui/code_editor/syntax_highlighter/vhdl_qss_adapter.h"

#include <QStyle>
namespace hal
{
    VhdlQssAdapter::VhdlQssAdapter(QWidget* parent) : QWidget(parent)
    {
        repolish();
        hide();
    }

    void VhdlQssAdapter::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        mKeywordFormat.setForeground(mKeywordColor);
        mKeywordFormat.setFontItalic(false);

        mTypeFormat.setForeground(mTypeColor);
        mTypeFormat.setFontItalic(false);

        mNumberFormat.setForeground(mNumberColor);
        mNumberFormat.setFontItalic(false);

        mStringFormat.setForeground(mStringColor);
        mStringFormat.setFontItalic(false);

        mCommentFormat.setForeground(mCommentColor);
        mCommentFormat.setFontItalic(false);
    }

    VhdlQssAdapter* VhdlQssAdapter::instance()
    {
        // CLEANUP NECESSARY ?
        static VhdlQssAdapter* instance = nullptr;

        if (!instance)
            instance = new VhdlQssAdapter();

        return instance;
    }

    QColor VhdlQssAdapter::textColor() const
    {
        return mTextColor;
    }

    QColor VhdlQssAdapter::keywordColor() const
    {
        return mKeywordColor;
    }

    QColor VhdlQssAdapter::typeColor() const
    {
        return mTypeColor;
    }

    QColor VhdlQssAdapter::numberColor() const
    {
        return mNumberColor;
    }

    QColor VhdlQssAdapter::stringColor() const
    {
        return mStringColor;
    }

    QColor VhdlQssAdapter::commentColor() const
    {
        return mCommentColor;
    }

    void VhdlQssAdapter::setTextColor(const QColor& color)
    {
        mTextColor = color;
    }

    void VhdlQssAdapter::setKeywordColor(const QColor& color)
    {
        mKeywordColor = color;
    }

    void VhdlQssAdapter::setTypeColor(const QColor& color)
    {
        mTypeColor = color;
    }

    void VhdlQssAdapter::setNumberColor(const QColor& color)
    {
        mNumberColor = color;
    }

    void VhdlQssAdapter::setStringColor(const QColor& color)
    {
        mStringColor = color;
    }

    void VhdlQssAdapter::setCommentColor(const QColor& color)
    {
        mCommentColor = color;
    }
}
