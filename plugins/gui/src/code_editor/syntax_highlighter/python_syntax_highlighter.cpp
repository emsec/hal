#include "gui/code_editor/syntax_highlighter/python_syntax_highlighter.h"

#include "gui/code_editor/syntax_highlighter/python_qss_adapter.h"
namespace hal
{
    PythonSyntaxHighlighter::PythonSyntaxHighlighter(QTextDocument* parent)
        : QSyntaxHighlighter(parent), mMultiLineCommentDelimiter("('''|\"\"\")"), mTrippleSingleQuote("'''"), mTrippleDoubleQuote("\"\"\"")
    //      //mKeywordColor(177, 240, 0),
    //      //mKeywordColor("#ffd200"),
    //      mKeywordColor("#CC7832"),
    //      //mKeywordColor("#6419E6"),
    //      //mKeywordColor("#9019E6"),
    //      //mOperatorColor("#0A81FF"),
    //      mOperatorColor("#A9B7C6"), mBraceColor("#A9B7C6"), mDefclassColor("#A9B7C6"),
    //      //mSingleQuotedStringColor("#17de4c"),
    //      //mDoubleQuotedStringColor("#17de4c"),
    //      mSingleQuotedStringColor("#A5C25C"), mDoubleQuotedStringColor("#A5C25C"),
    //      mSelfColor("#A9B7C6"),
    //      //mNumberColor(236, 118, 0),
    //      //mNumberColor("#DB8000"),
    //      //mNumberColor("#89F406"),
    //      //mNumberColor("#F4EC06"),
    //      mNumberColor("#6897BB"),
    //      //mCommentColor("#6F7B7B")
    //      //mCommentColor(222, 233, 175)
    //      //mCommentColor("#556062")
    //      //mCommentColor("#30474B") //2.0
    //      //mCommentColor("#3A565A") //2.5
    //      mCommentColor("#629755")    //2.98
    {
        HighlightingRule rule;

        rule.mPattern = QRegularExpression("\\b(and|as|assert|break|class|continue|def|del|elif|else|except|finally|for|from|global|if|import|in|is|lambda|"
                                          "nonlocal|not|or|pass|raise|return|try|while|with|yield|False|None|True)\\b");
        rule.mPattern.optimize();
        rule.mFormat = PythonQssAdapter::instance()->mKeywordFormat;
        mHighlightingRules.append(rule);

        rule.mPattern = QRegularExpression("=|==|!=|<|<=|>|>=|\\+|-|\\*|/|//|%|\\*\\*|\\+=|-=|\\*=|/=|%=|\\^|\\||&|~|>>|<<");
        rule.mPattern.optimize();
        rule.mFormat = PythonQssAdapter::instance()->mOperatorFormat;
        mHighlightingRules.append(rule);

        rule.mPattern = QRegularExpression("{|}|\\(|\\)|\\[|]");
        rule.mPattern.optimize();
        rule.mFormat = PythonQssAdapter::instance()->mBraceFormat;
        mHighlightingRules.append(rule);

        rule.mPattern = QRegularExpression("'[^']*'");
        rule.mPattern.optimize();
        rule.mFormat = PythonQssAdapter::instance()->mSingleQuotedStringFormat;
        mHighlightingRules.append(rule);

        rule.mPattern = QRegularExpression("\"[^\"]*\"");
        rule.mPattern.optimize();
        rule.mFormat = PythonQssAdapter::instance()->mDoubleQuotedStringFormat;
        mHighlightingRules.append(rule);

        rule.mPattern = QRegularExpression("\\bself\\b");
        rule.mPattern.optimize();
        rule.mFormat = PythonQssAdapter::instance()->mSelfFormat;
        mHighlightingRules.append(rule);

        rule.mPattern = QRegularExpression("(\\b([1-9][0-9]*|0)[.]?[0-9]*[eE][+-]?[0-9]+[jJ]?\\b)|"// Exponent float
                                          "(\\b([1-9][0-9]*|0)\\.[0-9]*[jJ]?)|"                           // Point float
                                          "(\\b0[xX][0-9A-Fa-f]+[lL]?\\b)|"                               // Hexadecimal integer
                                          "(\\b0[oO][0-7]+[lL]?\\b)|"                                     // Octal integer
                                          "(\\b([1-9][0-9]*|0)[lLjJ]?\\b)");                              // Decimal integer

        rule.mPattern.optimize();
        rule.mFormat = PythonQssAdapter::instance()->mNumberFormat;
        mHighlightingRules.append(rule);

        rule.mPattern = QRegularExpression("#[^\\n]*");
        rule.mPattern.optimize();
        rule.mFormat = PythonQssAdapter::instance()->mCommentFormat;
        mHighlightingRules.append(rule);
    }

    void PythonSyntaxHighlighter::highlightBlock(const QString& text)
    {
        for (const HighlightingRule& rule : mHighlightingRules)
        {
            QRegularExpressionMatchIterator it = rule.mPattern.globalMatch(text);
            while (it.hasNext())
            {
                QRegularExpressionMatch match = it.next();
                int index                     = match.capturedStart();
                int length                    = match.capturedLength();
                setFormat(index, length, rule.mFormat);
            }
        }

        // 0 no open comment in block
        // 1 contains tripple single quote start and no tripple single quote end OR follows a "1" block and also doesnt contain the end
        // 2 contains tripple double quote start and no tripple double quote end OR follows a "2" block and also doesnt contain the end

        int current_index = 0;

        if (previousBlockState() == 1)
        {
            current_index = closeMultilineComment(text, mTrippleSingleQuote);

            if (current_index < 0)
            {
                setCurrentBlockState(1);
                return;
            }
        }

        if (previousBlockState() == 2)
        {
            current_index = closeMultilineComment(text, mTrippleDoubleQuote);

            if (current_index < 0)
            {
                setCurrentBlockState(2);
                return;
            }
        }

        QRegularExpressionMatch match = mMultiLineCommentDelimiter.match(text, current_index);

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
                QRegularExpressionMatch single_match = mTrippleSingleQuote.match(text, comment_start_index + 3);
                int single_match_index               = single_match.capturedStart();

                if (single_match_index < 0)
                {
                    setCurrentBlockState(1);
                    setFormat(comment_start_index, text.length() - comment_start_index, PythonQssAdapter::instance()->mCommentFormat);
                    return;
                }
                else
                {
                    setFormat(comment_start_index, comment_start_index - single_match_index + 3, PythonQssAdapter::instance()->mCommentFormat);
                    match = mMultiLineCommentDelimiter.match(text, single_match_index + 3);
                }
            }
            else
            {
                QRegularExpressionMatch double_match = mTrippleDoubleQuote.match(text, comment_start_index + 3);
                int double_match_index               = double_match.capturedStart();

                if (double_match_index < 0)
                {
                    setCurrentBlockState(2);
                    setFormat(comment_start_index, text.length() - comment_start_index, PythonQssAdapter::instance()->mCommentFormat);
                    return;
                }
                else
                {
                    setFormat(comment_start_index, comment_start_index - double_match_index + 3, PythonQssAdapter::instance()->mCommentFormat);
                    match = mMultiLineCommentDelimiter.match(text, double_match_index + 3);
                }
            }
        }
        setCurrentBlockState(0);
        return;
    }

    int PythonSyntaxHighlighter::closeMultilineComment(const QString& text, const QRegularExpression& delimiter, const int offset)
    {
        QRegularExpressionMatch match = delimiter.match(text, offset);
        int index                     = match.capturedEnd();

        if (index < 0)
            setFormat(offset, text.length() - offset, PythonQssAdapter::instance()->mCommentFormat);
        else
            setFormat(offset, index - offset, PythonQssAdapter::instance()->mCommentFormat);

        return index;
    }
}
