#include "gui/code_editor/syntax_highlighter/vhdl_syntax_highlighter.h"

#include "gui/code_editor/syntax_highlighter/vhdl_qss_adapter.h"

namespace hal
{
    VhdlSyntaxHighlighter::VhdlSyntaxHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent)
    {
        HighlightingRule rule;

        rule.mPattern = QRegularExpression("(?<match>(\\b(abs|access|after|alias|all|and|architecture|array|assert|attribute|begin|block|body|buffer|bus|case|component|configuration|constant|disconnect|"
                                          "downto|else|elsif|end|entity|exit|file|for|function|generate|generic|group|guarded|if|impure|in|inertial|inout|is|label|library|linkage|literal|loop|map|mod|"
                                          "nand|new|next|nor|not|null|of|on|open|or|others|out|package|port|postponed|procedure|process|pure|range|record|register|reject|rem|report|return|rol|ror|select|"
                                          "severity|signal|shared|sla|sll|sra|srl|subtype|then|to|transport|type|unaffected|units|until|use|variable|wait|when|while|with|xnor|xor)\\b|=>|<=|:=))",
                                          QRegularExpression::CaseInsensitiveOption);
        rule.mPattern.optimize();
        rule.mFormat = VhdlQssAdapter::instance()->mKeywordFormat;
        mHighlightingRules.append(rule);

        rule.mPattern = QRegularExpression("(?<match>\\b[a-h,j-z]s\\b|\\bns\\b|rising_edge|falling_edge|TRUE|true|FALSE|false)");
        rule.mPattern.optimize();
        rule.mFormat = VhdlQssAdapter::instance()->mTypeFormat;
        mHighlightingRules.append(rule);

        rule.mPattern = QRegularExpression("(\\:\\s|in\\s|out\\s)(?<match>\\w*)(\\s?\\;|\\s:=|\\s\\()");
        rule.mPattern.optimize();
        rule.mFormat = VhdlQssAdapter::instance()->mTypeFormat;
        mHighlightingRules.append(rule);

        rule.mPattern = QRegularExpression("(\\:\\s+)(?<match>\\b.+\\b)(\\s*)$");
        rule.mPattern.optimize();
        rule.mFormat = VhdlQssAdapter::instance()->mKeywordFormat;
        mHighlightingRules.append(rule);

        rule.mPattern = QRegularExpression("(?<match>'.+'|\\b'?\\d+'?\\b)");
        rule.mPattern.optimize();
        rule.mFormat = VhdlQssAdapter::instance()->mNumberFormat;
        mHighlightingRules.append(rule);

        rule.mPattern = QRegularExpression("(?<match>\"\\D.*\")");
        rule.mPattern.optimize();
        rule.mFormat = VhdlQssAdapter::instance()->mStringFormat;
        mHighlightingRules.append(rule);

        rule.mPattern = QRegularExpression("(?<match>\\bX\".*\")");
        rule.mPattern.optimize();
        rule.mFormat = VhdlQssAdapter::instance()->mNumberFormat;
        mHighlightingRules.append(rule);

        mSingleLineCommentExpression = QRegularExpression("(?<match>--[^\n]*)");
        mCommentStartExpression       = QRegularExpression("(/\\*)");
        mCommentEndExpression         = QRegularExpression("(\\*/)");
    }

    void VhdlSyntaxHighlighter::highlightBlock(const QString& text)
    {
        for (const HighlightingRule& rule : mHighlightingRules)
        {
            QRegularExpressionMatchIterator it = rule.mPattern.globalMatch(text);
            while (it.hasNext())
            {
                QRegularExpressionMatch match = it.next();
                int index                     = match.capturedStart("match");
                int length                    = match.capturedLength("match");
                setFormat(index, length, rule.mFormat);
            }
        }

        QRegularExpressionMatchIterator it = mSingleLineCommentExpression.globalMatch(text);
        while (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            int index                     = match.capturedStart("match");
            int length                    = match.capturedLength("match");
            setFormat(index, length, VhdlQssAdapter::instance()->mCommentFormat);
        }

        setCurrentBlockState(0);
        int start_index = 0;

        if (previousBlockState() != 1)
        {
            QRegularExpressionMatch match = mCommentStartExpression.match(text);
            if (match.hasMatch())
                start_index = match.capturedStart(0);
            else
                start_index = -1;
        }

        while (start_index >= 0)
        {
            int end_index                 = 0;
            QRegularExpressionMatch match = mCommentEndExpression.match(text, start_index);
            if (match.hasMatch())
                end_index = match.capturedStart(0);
            else
                end_index = -1;
            int comment_length;
            if (end_index == -1)
            {
                setCurrentBlockState(1);
                comment_length = text.length() - start_index;
            }
            else
            {
                comment_length = end_index - start_index + match.capturedLength(0);
            }
            setFormat(start_index, comment_length, VhdlQssAdapter::instance()->mCommentFormat);
            match = mCommentStartExpression.match(text, start_index + comment_length);
            if (match.hasMatch())
                start_index = match.capturedStart(0);
            else
                start_index = -1;
        }
    }
}
