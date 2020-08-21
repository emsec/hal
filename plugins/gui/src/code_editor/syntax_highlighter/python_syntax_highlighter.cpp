#include "gui/code_editor/syntax_highlighter/python_syntax_highlighter.h"

#include "gui/code_editor/syntax_highlighter/python_qss_adapter.h"
namespace hal
{
    PythonSyntaxHighlighter::PythonSyntaxHighlighter(QTextDocument* parent)
        : QSyntaxHighlighter(parent), m_multi_line_comment_delimiter("('''|\"\"\")"), m_tripple_single_quote("'''"), m_tripple_double_quote("\"\"\"")
    //      //m_keyword_color(177, 240, 0),
    //      //m_keyword_color("#ffd200"),
    //      m_keyword_color("#CC7832"),
    //      //m_keyword_color("#6419E6"),
    //      //m_keyword_color("#9019E6"),
    //      //m_operator_color("#0A81FF"),
    //      m_operator_color("#A9B7C6"), m_brace_color("#A9B7C6"), m_defclass_color("#A9B7C6"),
    //      //m_single_quoted_string_color("#17de4c"),
    //      //m_double_quoted_string_color("#17de4c"),
    //      m_single_quoted_string_color("#A5C25C"), m_double_quoted_string_color("#A5C25C"),
    //      m_self_color("#A9B7C6"),
    //      //m_number_color(236, 118, 0),
    //      //m_number_color("#DB8000"),
    //      //m_number_color("#89F406"),
    //      //m_number_color("#F4EC06"),
    //      m_number_color("#6897BB"),
    //      //m_comment_color("#6F7B7B")
    //      //m_comment_color(222, 233, 175)
    //      //m_comment_color("#556062")
    //      //m_comment_color("#30474B") //2.0
    //      //m_comment_color("#3A565A") //2.5
    //      m_comment_color("#629755")    //2.98
    {
        highlighting_rule rule;

        rule.pattern = QRegularExpression("\\b(and|as|assert|break|class|continue|def|del|elif|else|except|finally|for|from|global|if|import|in|is|lambda|"
                                          "nonlocal|not|or|pass|raise|return|try|while|with|yield|False|None|True)\\b");
        rule.pattern.optimize();
        rule.format = PythonQssAdapter::instance()->m_keyword_format;
        m_highlighting_rules.append(rule);

        rule.pattern = QRegularExpression("=|==|!=|<|<=|>|>=|\\+|-|\\*|/|//|%|\\*\\*|\\+=|-=|\\*=|/=|%=|\\^|\\||&|~|>>|<<");
        rule.pattern.optimize();
        rule.format = PythonQssAdapter::instance()->m_operator_format;
        m_highlighting_rules.append(rule);

        rule.pattern = QRegularExpression("{|}|\\(|\\)|\\[|]");
        rule.pattern.optimize();
        rule.format = PythonQssAdapter::instance()->m_brace_format;
        m_highlighting_rules.append(rule);

        rule.pattern = QRegularExpression("'[^']*'");
        rule.pattern.optimize();
        rule.format = PythonQssAdapter::instance()->m_single_quoted_string_format;
        m_highlighting_rules.append(rule);

        rule.pattern = QRegularExpression("\"[^\"]*\"");
        rule.pattern.optimize();
        rule.format = PythonQssAdapter::instance()->m_double_quoted_string_format;
        m_highlighting_rules.append(rule);

        rule.pattern = QRegularExpression("\\bself\\b");
        rule.pattern.optimize();
        rule.format = PythonQssAdapter::instance()->m_self_format;
        m_highlighting_rules.append(rule);

        rule.pattern = QRegularExpression("(\\b([1-9][0-9]*|0)\\.[0-9]*[eE][+-]?[0-9]+\\b)|"    // Exponent float
                                          "(\\b([1-9][0-9]*|0)\\.[0-9]*)|"                      // Point float
                                          "(\\b0[xX][0-9A-Fa-f]+[lL]?\\b)|"                     // Hexadecimal integer
                                          "(\\b0[0-7]+[lL]?\\b)|"                               // Octal integer
                                          "(\\b([1-9][0-9]*|0)[lL]?\\b)");                      // Decimal integer

        rule.pattern.optimize();
        rule.format = PythonQssAdapter::instance()->m_number_format;
        m_highlighting_rules.append(rule);

        rule.pattern = QRegularExpression("#[^\\n]*");
        rule.pattern.optimize();
        rule.format = PythonQssAdapter::instance()->m_comment_format;
        m_highlighting_rules.append(rule);
    }

    void PythonSyntaxHighlighter::highlightBlock(const QString& text)
    {
        for (const highlighting_rule& rule : m_highlighting_rules)
        {
            QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
            while (it.hasNext())
            {
                QRegularExpressionMatch match = it.next();
                int index                     = match.capturedStart();
                int length                    = match.capturedLength();
                setFormat(index, length, rule.format);
            }
        }

        // 0 no open comment in block
        // 1 contains tripple single quote start and no tripple single quote end OR follows a "1" block and also doesnt contain the end
        // 2 contains tripple double quote start and no tripple double quote end OR follows a "2" block and also doesnt contain the end

        int current_index = 0;

        if (previousBlockState() == 1)
        {
            current_index = close_multiline_comment(text, m_tripple_single_quote);

            if (current_index < 0)
            {
                setCurrentBlockState(1);
                return;
            }
        }

        if (previousBlockState() == 2)
        {
            current_index = close_multiline_comment(text, m_tripple_double_quote);

            if (current_index < 0)
            {
                setCurrentBlockState(2);
                return;
            }
        }

        QRegularExpressionMatch match = m_multi_line_comment_delimiter.match(text, current_index);

        while (match.hasMatch())
        {
            // check which delimiter was found
            // check for closing delimiter
            // found -> loop again
            // not found -> set block state and return

            int comment_start_index = match.capturedStart();
            QChar comment_type      = text.at(comment_start_index);

            if (comment_type == '\'')
            {
                QRegularExpressionMatch single_match = m_tripple_single_quote.match(text, comment_start_index + 3);
                int single_match_index               = single_match.capturedStart();

                if (single_match_index < 0)
                {
                    setCurrentBlockState(1);
                    setFormat(comment_start_index, text.length() - comment_start_index, PythonQssAdapter::instance()->m_comment_format);
                    return;
                }
                else
                {
                    setFormat(comment_start_index, comment_start_index - single_match_index + 3, PythonQssAdapter::instance()->m_comment_format);
                    match = m_multi_line_comment_delimiter.match(text, single_match_index + 3);
                }
            }
            else
            {
                QRegularExpressionMatch double_match = m_tripple_double_quote.match(text, comment_start_index + 3);
                int double_match_index               = double_match.capturedStart();

                if (double_match_index < 0)
                {
                    setCurrentBlockState(2);
                    setFormat(comment_start_index, text.length() - comment_start_index, PythonQssAdapter::instance()->m_comment_format);
                    return;
                }
                else
                {
                    setFormat(comment_start_index, comment_start_index - double_match_index + 3, PythonQssAdapter::instance()->m_comment_format);
                    match = m_multi_line_comment_delimiter.match(text, double_match_index + 3);
                }
            }
        }
        setCurrentBlockState(0);
        return;
    }

    int PythonSyntaxHighlighter::close_multiline_comment(const QString& text, const QRegularExpression& delimiter, const int offset)
    {
        QRegularExpressionMatch match = delimiter.match(text, offset);
        int index                     = match.capturedEnd();

        if (index < 0)
            setFormat(offset, text.length() - offset, PythonQssAdapter::instance()->m_comment_format);
        else
            setFormat(offset, index - offset, PythonQssAdapter::instance()->m_comment_format);

        return index;
    }
}
