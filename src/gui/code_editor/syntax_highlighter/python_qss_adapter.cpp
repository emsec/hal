#include "code_editor/syntax_highlighter/python_qss_adapter.h"

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

        m_keyword_format.setForeground(m_keyword_color);
        m_keyword_format.setFontItalic(false);

        m_operator_format.setForeground(m_operator_color);
        m_operator_format.setFontItalic(false);

        m_brace_format.setForeground(m_brace_color);
        m_brace_format.setFontItalic(false);

        m_defclass_format.setForeground(m_defclass_color);
        m_defclass_format.setFontItalic(false);

        m_self_format.setForeground(m_self_color);
        m_self_format.setFontItalic(false);

        m_number_format.setForeground(m_number_color);
        m_number_format.setFontItalic(false);

        m_single_quoted_string_format.setForeground(m_single_quoted_string_color);
        m_single_quoted_string_format.setFontItalic(false);

        m_double_quoted_string_format.setForeground(m_double_quoted_string_color);
        m_double_quoted_string_format.setFontItalic(false);

        m_comment_format.setForeground(m_comment_color);
        m_comment_format.setFontItalic(false);
    }

    PythonQssAdapter* PythonQssAdapter::instance()
    {
        // CLEANUP NECESSARY ?
        static PythonQssAdapter* instance = nullptr;

        if (!instance)
            instance = new PythonQssAdapter();

        return instance;
    }

    QColor PythonQssAdapter::text_color() const
    {
        return m_text_color;
    }

    QColor PythonQssAdapter::keyword_color() const
    {
        return m_keyword_color;
    }

    QColor PythonQssAdapter::operator_color() const
    {
        return m_operator_color;
    }

    QColor PythonQssAdapter::brace_color() const
    {
        return m_brace_color;
    }

    QColor PythonQssAdapter::defclass_color() const
    {
        return m_defclass_color;
    }

    QColor PythonQssAdapter::self_color() const
    {
        return m_self_color;
    }

    QColor PythonQssAdapter::number_color() const
    {
        return m_number_color;
    }

    QColor PythonQssAdapter::single_quoted_string_color() const
    {
        return m_single_quoted_string_color;
    }

    QColor PythonQssAdapter::double_quoted_string_color() const
    {
        return m_double_quoted_string_color;
    }

    QColor PythonQssAdapter::comment_color() const
    {
        return m_comment_color;
    }

    void PythonQssAdapter::set_text_color(const QColor& color)
    {
        m_text_color = color;
    }

    void PythonQssAdapter::set_keyword_color(const QColor& color)
    {
        m_keyword_color = color;
    }

    void PythonQssAdapter::set_operator_color(const QColor& color)
    {
        m_operator_color = color;
    }

    void PythonQssAdapter::set_brace_color(const QColor& color)
    {
        m_brace_color = color;
    }

    void PythonQssAdapter::set_defclass_color(const QColor& color)
    {
        m_defclass_color = color;
    }

    void PythonQssAdapter::set_self_color(const QColor& color)
    {
        m_self_color = color;
    }

    void PythonQssAdapter::set_number_color(const QColor& color)
    {
        m_number_color = color;
    }

    void PythonQssAdapter::set_single_quoted_string_color(const QColor& color)
    {
        m_single_quoted_string_color = color;
    }

    void PythonQssAdapter::set_double_quoted_string_color(const QColor& color)
    {
        m_double_quoted_string_color = color;
    }

    void PythonQssAdapter::set_comment_color(const QColor& color)
    {
        m_comment_color = color;
    }
}
