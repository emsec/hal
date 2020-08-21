#include "code_editor/syntax_highlighter/vhdl_qss_adapter.h"

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

    VhdlQssAdapter* VhdlQssAdapter::instance()
    {
        // CLEANUP NECESSARY ?
        static VhdlQssAdapter* instance = nullptr;

        if (!instance)
            instance = new VhdlQssAdapter();

        return instance;
    }

    QColor VhdlQssAdapter::text_color() const
    {
        return m_text_color;
    }

    QColor VhdlQssAdapter::keyword_color() const
    {
        return m_keyword_color;
    }

    QColor VhdlQssAdapter::type_color() const
    {
        return m_type_color;
    }

    QColor VhdlQssAdapter::number_color() const
    {
        return m_number_color;
    }

    QColor VhdlQssAdapter::string_color() const
    {
        return m_string_color;
    }

    QColor VhdlQssAdapter::comment_color() const
    {
        return m_comment_color;
    }

    void VhdlQssAdapter::set_text_color(const QColor& color)
    {
        m_text_color = color;
    }

    void VhdlQssAdapter::set_keyword_color(const QColor& color)
    {
        m_keyword_color = color;
    }

    void VhdlQssAdapter::set_type_color(const QColor& color)
    {
        m_type_color = color;
    }

    void VhdlQssAdapter::set_number_color(const QColor& color)
    {
        m_number_color = color;
    }

    void VhdlQssAdapter::set_string_color(const QColor& color)
    {
        m_string_color = color;
    }

    void VhdlQssAdapter::set_comment_color(const QColor& color)
    {
        m_comment_color = color;
    }
}
