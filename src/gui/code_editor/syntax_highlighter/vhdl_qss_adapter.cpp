#include "code_editor/syntax_highlighter/vhdl_qss_adapter.h"

#include <QStyle>
namespace hal
{
    vhdl_qss_adapter::vhdl_qss_adapter(QWidget* parent) : QWidget(parent)
    {
        repolish();
        hide();
    }

    void vhdl_qss_adapter::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        m_keyword_format.setForeground(m_keyword_color);
        m_keyword_format.setFontItalic(false);

        m_type_format.setForeground(m_type_color);
        m_type_format.setFontItalic(false);

        m_number_format.setForeground(m_number_color);
        m_number_format.setFontItalic(false);

        m_string_format.setForeground(m_string_color);
        m_string_format.setFontItalic(false);

        m_comment_format.setForeground(m_comment_color);
        m_comment_format.setFontItalic(false);
    }

    vhdl_qss_adapter* vhdl_qss_adapter::instance()
    {
        // CLEANUP NECESSARY ?
        static vhdl_qss_adapter* instance = nullptr;

        if (!instance)
            instance = new vhdl_qss_adapter();

        return instance;
    }

    QColor vhdl_qss_adapter::text_color() const
    {
        return m_text_color;
    }

    QColor vhdl_qss_adapter::keyword_color() const
    {
        return m_keyword_color;
    }

    QColor vhdl_qss_adapter::type_color() const
    {
        return m_type_color;
    }

    QColor vhdl_qss_adapter::number_color() const
    {
        return m_number_color;
    }

    QColor vhdl_qss_adapter::string_color() const
    {
        return m_string_color;
    }

    QColor vhdl_qss_adapter::comment_color() const
    {
        return m_comment_color;
    }

    void vhdl_qss_adapter::set_text_color(const QColor& color)
    {
        m_text_color = color;
    }

    void vhdl_qss_adapter::set_keyword_color(const QColor& color)
    {
        m_keyword_color = color;
    }

    void vhdl_qss_adapter::set_type_color(const QColor& color)
    {
        m_type_color = color;
    }

    void vhdl_qss_adapter::set_number_color(const QColor& color)
    {
        m_number_color = color;
    }

    void vhdl_qss_adapter::set_string_color(const QColor& color)
    {
        m_string_color = color;
    }

    void vhdl_qss_adapter::set_comment_color(const QColor& color)
    {
        m_comment_color = color;
    }
}
