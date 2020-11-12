#include "gui/code_editor/syntax_highlighter/vhdl_syntax_highlighter.h"

#include "gui/code_editor/syntax_highlighter/vhdl_qss_adapter.h"

namespace hal
{
    VhdlSyntaxHighlighter::VhdlSyntaxHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent)
    {
        //        m_pattern_options = QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption;
        //        m_match_type = QRegularExpression::NormalMatch;
        //        m_match_options = QRegularExpression::AnchoredMatchOption;
        //        m_current_state = "root";
        //        m_valid_identifier << "Text" << "Comment" << "Comment.Single" << "String" << "String.Char" << "Operator"
        //                           << "Name" << "Name.Attribute" << "Name.Class" << "Name.Namespace" << "Punctuation"
        //                           << "Keyword" << "Keyword.Type" << "Number" << "Number.Integer" << "Number.Float"
        //                           << "Number.Hex" << "Number.Oct" << "Number.Bin";
        //
        //
        //        m_formatters["Text"] = QTextCharFormat();
        //        m_formatters["Text"].setForeground(QColor(228, 228, 228));
        //        m_formatters["Comment"] = QTextCharFormat();
        //        m_formatters["Comment"].setForeground(QColor(128, 128, 128));
        //        m_formatters["Comment.Single"] = m_formatters["Comment"];
        //        m_formatters["Comment.Multi"] = m_formatters["Comment"];
        //        m_formatters["String"] = QTextCharFormat();
        //        m_formatters["String"].setForeground(QColor(230, 220, 108));
        //        m_formatters["String.Char"] = m_formatters["String"];
        //        m_formatters["Operator"] = QTextCharFormat();
        //        m_formatters["Operator"].setForeground(QColor(96, 217, 241));
        //        m_formatters["Name"] = QTextCharFormat();
        //        m_formatters["Name"].setForeground(QColor(252, 30, 112));
        //        m_formatters["Name.Attribute"] = m_formatters["Name"];
        //        m_formatters["Name.Class"] = m_formatters["Name"];
        //        m_formatters["Name.Namespace"] = m_formatters["Name"];
        //        m_formatters["Punctuation"] = m_formatters["Text"];
        //        m_formatters["Keyword"] = QTextCharFormat();
        //        m_formatters["Keyword"].setForeground(QColor(252, 30, 112));
        //        m_formatters["Keyword.Type"] = m_formatters["Keyword"];
        //        m_formatters["Number"] = QTextCharFormat();
        //        m_formatters["Number"].setForeground(QColor(174, 125, 255));
        //        m_formatters["Number.Integer"] = m_formatters["Number"];
        //        m_formatters["Number.Float"] = m_formatters["Number"];
        //        m_formatters["Number.Hex"] = m_formatters["Number"];
        //        m_formatters["Number.Oct"] = m_formatters["Number"];
        //        m_formatters["Number.Bin"] = m_formatters["Number"];
        //
        //        m_types << "\\bboolean\\b" <<  "\\bbit\\b" <<  "\\bcharacter\\b" <<  "\\bseverity_level\\b" <<  "\\binteger\\b"
        //                <<  "\\btime\\b" << "\\bdelay_length\\b" <<  "\\bnatural\\b" <<  "\\bpositive\\b" <<  "\\bstring\\b"
        //                <<  "\\bbit_vector\\b" << "\\bfile_open_kind\\b" <<  "\\bfile_open_status\\b" <<  "\\bstd_ulogic\\b"
        //                <<  "\\bstd_ulogic_vector\\b" << "\\bstd_logic\\b" <<  "\\bstd_logic_vector\\b";
        //
        //        QString types_pattern = QString("(" + m_types.join('|') + ")");
        //        m_type_rule.expression = QRegularExpression(types_pattern, m_pattern_options);
        //        m_type_rule.expression.optimize();
        //        m_type_rule.list << "Keyword.Type";
        //        m_type_rule.state = "";
        //
        //        mKeywords << "\\babs\\b" << "\\baccess\\b" << "\\bafter\\b" << "\\balias\\b" << "\\ball\\b" << "\\band\\b"
        //                        << "\\barchitecture\\b" << "\\barray\\b" << "\\bassert\\b" << "\\battribute\\b" << "\\bbegin\\b"
        //                        << "\\bblock\\b" << "\\bbody\\b" << "\\bbuffer\\b" << "\\bbus\\b" << "\\bcase\\b"
        //                        << "\\bcomponent\\b" << "\\bconfiguration\\b" << "\\bconstant\\b" << "\\bdisconnect\\b"
        //                        << "\\bdownto\\b" << "\\belse\\b" << "\\belsif\\b" << "\\bend\\b" << "\\bentity\\b"
        //                        << "\\bexit\\b" << "\\bfile\\b" << "\\bfor\\b" << "\\bfunction\\b" << "\\bgenerate\\b"
        //                        << "\\bgeneric\\b" << "\\bgroup\\b" << "\\bguarded\\b" << "\\bif\\b" << "\\bimpure\\b"
        //                        << "\\bin\\b" << "\\binertial\\b" << "\\binout\\b" << "\\bis\\b" << "\\blabel\\b"
        //                        << "\\blibrary\\b" << "\\blinkage\\b" << "\\bliteral\\b" << "\\bloop\\b" << "\\bmap\\b"
        //                        << "\\bmod\\b" << "\\bnand\\b" << "\\bnew\\b" << "\\bnext\\b" << "\\bnor\\b" << "\\bnot\\b"
        //                        << "\\bnull\\b" << "\\bof\\b" << "\\bon\\b" << "\\bopen\\b" << "\\bor\\b" << "\\bothers\\b"
        //                        << "\\bout\\b" << "\\bpackage\\b" << "\\bport\\b" << "\\bpostponed\\b" << "\\bprocedure\\b"
        //                        << "\\bprocess\\b" << "\\bpure\\b" << "\\brange\\b" << "\\brecord\\b" << "\\bregister\\b"
        //                        << "\\breject\\b" << "\\breturn\\b" << "\\brol\\b" << "\\bror\\b" << "\\bselect\\b"
        //                        << "\\bseverity\\b" << "\\bsignal\\b" << "\\bshared\\b" << "\\bsla\\b" << "\\bsli\\b"
        //                        << "\\bsra\\b" << "\\bsrl\\b" << "\\bsubtype\\b" << "\\bthen\\b" << "\\bto\\b"
        //                        << "\\btransport\\b" << "\\btype\\b" << "\\bunits\\b" << "\\buntil\\b" << "\\buse\\b"
        //                        << "\\bvariable\\b" << "\\bwait\\b" << "\\bwhen\\b" << "\\bwhile\\b" << "\\bwith\\b"
        //                        << "\\bxnor\\b" << "\\bxor\\b";
        //
        //        QString keywords_pattern = QString("(" + mKeywords.join('|') + ")");
        //        m_keyword_rule.expression = QRegularExpression(keywords_pattern, m_pattern_options);
        //        m_keyword_rule.expression.optimize();
        //        m_keyword_rule.list << "Keyword";
        //        m_keyword_rule.state = "";
        //
        //        m_rules["root"] = QList<HighlightingRule>();
        //        m_rules["root"].append(make_rule("\\n", QStringList() << "Text", ""));
        //        m_rules["root"].append(make_rule("\\s+", QStringList() << "Text", ""));
        //        m_rules["root"].append(make_rule("\\\\\\n", QStringList() << "Text", ""));
        //        m_rules["root"].append(make_rule("/\\*", QStringList() << "Comment.Multi", "comment"));
        //        m_rules["root"].append(make_rule("--.*?$", QStringList() << "Comment.Single", ""));
        //        m_rules["root"].append(make_rule("'(U|X|0|1|Z|W|L|H|-)'", QStringList() << "String.Char", ""));
        //        m_rules["root"].append(make_rule("(\\b=>\\b|\\b<=\\b|\\b:=\\b|[~!%^&*+=|?:<>/-])", QStringList() << "Operator", ""));
        //        m_rules["root"].append(make_rule("'[a-z_]\\w*", QStringList() << "Name.Attribute", ""));
        //        m_rules["root"].append(make_rule("[()\\[\\],.;\\']", QStringList() << "Punctuation", ""));
        //        m_rules["root"].append(make_rule("\"[^\\n\\\\]*\"", QStringList() << "String", ""));
        //        m_rules["root"].append(make_rule("(library)(\\s+)([a-z_]\\w*)", QStringList() << "Keyword" << "Text" << "Name.Namespace", ""));
        //        m_rules["root"].append(make_rule("(use)(\\s+)(entity)", QStringList() << "Keyword" << "Text" << "Keyword", ""));
        //        m_rules["root"].append(make_rule("(use)(\\s+)([a-z_][\\w.]*)", QStringList() << "Keyword" << "Text" << "Name.Namespace", ""));
        //        m_rules["root"].append(make_rule("(entity|component)(\\s+)([a-z_]\\w*)", QStringList() << "Keyword" << "Text" << "Name.Class", ""));
        //        m_rules["root"].append(make_rule("(architecture|configuration)(\\s+)([a-z_]\\w*)(\\s+)(of)(\\s+)([a-z_]\\w*)(\\s+)(is)", QStringList() << "Keyword" << "Text" << "Name.Class" << "Text" << "Keyword" << "Text" << "Name.Class" << "Text" << "Keyword", ""));
        //        m_rules["root"].append(make_rule("(end)(\\s+)", QStringList() << "Keyword" << "Text", "endblock"));
        //        m_rules["root"].append(m_type_rule);
        //        m_rules["root"].append(m_keyword_rule);
        //        m_rules["root"].append(make_rule("\\b\\d{1,2}#[0-9a-f_]+#?", QStringList() << "Number.Integer", ""));
        //        m_rules["root"].append(make_rule("\\b\\d+", QStringList() << "Number.Integer", ""));
        //        m_rules["root"].append(make_rule("(\\d+\\.\\d*|\\.\\d+|\\d+)E[+-]?\\d+", QStringList() << "Number.Float", ""));
        //        m_rules["root"].append(make_rule("X\"[0-9a-f_]+\"", QStringList() << "Number.Hex", ""));
        //        m_rules["root"].append(make_rule("O\"[0-7_]+\"", QStringList() << "Number.Oct", ""));
        //        m_rules["root"].append(make_rule("bB\"[01_]+\"", QStringList() << "Number.Bin", ""));
        //
        //        m_rules["comment"] = QList<HighlightingRule>();
        //        m_rules["comment"].append(make_rule("[^*/]", QStringList() << "Comment.Multi", ""));
        //        m_rules["comment"].append(make_rule("/\\*", QStringList() << "Comment.Multi", "#push"));
        //        m_rules["comment"].append(make_rule("\\*/", QStringList() << "Comment.Multi", "#pop"));
        //        m_rules["comment"].append(make_rule("[*/]", QStringList() << "Comment.Multi", ""));
        //
        //        m_rules["endblock"] = QList<HighlightingRule>();
        //        m_rules["endblock"].append(m_keyword_rule);
        //        m_rules["endblock"].append(make_rule("[a-z_]\\w*", QStringList() << "Name.Class", ""));
        //        m_rules["endblock"].append(make_rule("(\\s+)", QStringList() << "Text", ""));
        //        m_rules["endblock"].append(make_rule(";", QStringList() << "Punctuation", "#pop"));

        // NEW STUFF HERE

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
        //this->run_current_state(text);
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

//VhdlSyntaxHighlighter::HighlightingRule VhdlSyntaxHighlighter::make_rule(QString expression,
//                                                                              QStringList list, QString state)
//{
//        HighlightingRule rule;
//        rule.expression = QRegularExpression(expression, m_pattern_options);
//        rule.expression.optimize();
//        rule.list = list;
//        rule.state = state;
//        return rule;
//}
//
//void VhdlSyntaxHighlighter::run_current_state(const QString &text, int offset)
//{
//        if (!m_rules.contains(m_current_state)) {
//                log_error("Current state not found in syntax highlighter!\n");
//                return;
//        }
//        bool rerun = false;
//        QList<HighlightingRule> rule_set = m_rules[m_current_state];
//        int max = text.length() - 1;
//        for (const HighlightingRule &rule : rule_set) {
//                QRegularExpressionMatch match = rule.expression.match(text, offset, m_match_type, m_match_options);
//                if (match.hasMatch()) {
//                        if (rule.list.length() == 1) {
//                                // Single argument match
//                                setFormat(match.capturedStart(0), match.capturedLength(0), m_formatters[rule.list.at(0)]);
//                        } else if (rule.list.length() >= 1){
//                                // Multi argument match
//                                for (int i = 1; i <= match.lastCapturedIndex(); ++i) {
//                                        setFormat(match.capturedStart(i), match.capturedLength(i), m_formatters[rule.list.at(i-1)]);
//                                }
//                        } else {
//                                log_error("No formatting option for captured regexp set!\n");
//                        }
//                        offset = offset + match.capturedLength(0);
//                        //Select next state
//                        if (rule.state.isEmpty()) {
//                                continue;
//                        } else {
//                                if (rule.state.startsWith("#")) {
//                                        //push or pop
//                                        if (rule.state.contains("#push")) {
//                                                m_stack.push(m_current_state);
//                                                rerun = true;
//                                                break;
//                                        } else if (rule.state.contains("#pop")) {
//                                                if (m_stack.isEmpty()) {
//                                                        m_current_state = "root";
//                                                        rerun = true;
//                                                        break;
//                                                } else {
//                                                        QString state = m_stack.pop();
//                                                        if (state.compare(m_current_state) != 0) {
//                                                                m_current_state = state;
//                                                                rerun = true;
//                                                                break;
//                                                        }
//                                                }
//                                        } else {
//                                                log_error("Unknown next state format!\n");
//                                        }
//                                } else {
//                                        m_current_state = rule.state;
//                                        rerun = true;
//                                        break;
//                                }
//                        }
//                }
//        }
//        if (rerun) {
//                this->run_current_state(text, offset);
//        }
//        if (offset < max) {
//                //No match and not all processed set offset +1 and rerun
//                ++offset;
//                this->run_current_state(text, offset);
//        }
//}
