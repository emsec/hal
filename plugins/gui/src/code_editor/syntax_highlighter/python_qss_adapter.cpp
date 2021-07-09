#include "gui/code_editor/syntax_highlighter/python_qss_adapter.h"

#include <QDebug>
#include <QStyle>
namespace hal
{
    PythonQssAdapter::PythonQssAdapter(QWidget* parent) : QWidget(parent)
    {
        repolish();
        hide();
    }

    PythonQssAdapter::~PythonQssAdapter()
    {
        qDebug() << "python qss adapter killed"; // DEBUG CODE, DELETE LATER
    }

    void PythonQssAdapter::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        mKeywordFormat.setForeground(mKeywordColor);
        mKeywordFormat.setFontItalic(false);

        mOperatorFormat.setForeground(mOperatorColor);
        mOperatorFormat.setFontItalic(false);

        mBraceFormat.setForeground(mBraceColor);
        mBraceFormat.setFontItalic(false);

        mDefclassFormat.setForeground(mDefclassColor);
        mDefclassFormat.setFontItalic(false);

        mSelfFormat.setForeground(mSelfColor);
        mSelfFormat.setFontItalic(false);

        mNumberFormat.setForeground(mNumberColor);
        mNumberFormat.setFontItalic(false);

        mSingleQuotedStringFormat.setForeground(mSingleQuotedStringColor);
        mSingleQuotedStringFormat.setFontItalic(false);

        mDoubleQuotedStringFormat.setForeground(mDoubleQuotedStringColor);
        mDoubleQuotedStringFormat.setFontItalic(false);

        mCommentFormat.setForeground(mCommentColor);
        mCommentFormat.setFontItalic(false);
    }

    PythonQssAdapter* PythonQssAdapter::instance()
    {
        // CLEANUP NECESSARY ?
        static PythonQssAdapter* instance = nullptr;

        if (!instance)
            instance = new PythonQssAdapter();

        return instance;
    }

    QColor PythonQssAdapter::keywordColor() const
    {
        return mKeywordColor;
    }

    QColor PythonQssAdapter::operator_color() const
    {
        return mOperatorColor;
    }

    QColor PythonQssAdapter::braceColor() const
    {
        return mBraceColor;
    }

    QColor PythonQssAdapter::defclassColor() const
    {
        return mDefclassColor;
    }

    QColor PythonQssAdapter::selfColor() const
    {
        return mSelfColor;
    }

    QColor PythonQssAdapter::numberColor() const
    {
        return mNumberColor;
    }

    QColor PythonQssAdapter::singleQuotedStringColor() const
    {
        return mSingleQuotedStringColor;
    }

    QColor PythonQssAdapter::doubleQuotedStringColor() const
    {
        return mDoubleQuotedStringColor;
    }

    QColor PythonQssAdapter::commentColor() const
    {
        return mCommentColor;
    }

    /*void PythonQssAdapter::setTextColor(const QColor& color)
    {
        mTextColor = color;
    }*/

    void PythonQssAdapter::setKeywordColor(const QColor& color)
    {
        mKeywordColor = color;
    }

    void PythonQssAdapter::setOperatorColor(const QColor& color)
    {
        mOperatorColor = color;
    }

    void PythonQssAdapter::setBraceColor(const QColor& color)
    {
        mBraceColor = color;
    }

    void PythonQssAdapter::setDefclassColor(const QColor& color)
    {
        mDefclassColor = color;
    }

    void PythonQssAdapter::setSelfColor(const QColor& color)
    {
        mSelfColor = color;
    }

    void PythonQssAdapter::setNumberColor(const QColor& color)
    {
        mNumberColor = color;
    }

    void PythonQssAdapter::setSingleQuotedStringColor(const QColor& color)
    {
        mSingleQuotedStringColor = color;
    }

    void PythonQssAdapter::setDoubleQuotedStringColor(const QColor& color)
    {
        mDoubleQuotedStringColor = color;
    }

    void PythonQssAdapter::setCommentColor(const QColor& color)
    {
        mCommentColor = color;
    }
}
