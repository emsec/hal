#include "gui/comment_system/widgets/comment_item.h"
#include "gui/comment_system/comment_entry.h"
#include "gui/comment_system/widgets/comment_dialog.h"
#include "gui/searchbar/searchable_label.h"
#include "gui/gui_globals.h"

#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QAction>
#include <QToolButton>
#include <QDebug>
#include "gui/gui_utils/graphics.h"
#if QT_VERSION >= QT_VERSION_CHECK(5,13,0)
   #include <QRegularExpression>
#else
   #include <QRegExp>
#endif

namespace hal
{
    CommentItem::CommentItem(QWidget* parent) : QFrame(parent)
    {
        init();
    }

    CommentItem::CommentItem(CommentEntry *entry, QWidget *parent) : QFrame(parent)
    {
        init();
        mEntry = entry;
        updateCurrentEntry();
    }

    CommentItem::~CommentItem()
    {
    }

    void CommentItem::setComment(CommentEntry *entry)
    {
        mEntry = entry;
        updateCurrentEntry();
    }

    void CommentItem::updateCurrentEntry()
    {
        if(!mEntry) return;
        mHeader->setText(mEntry->getHeader());
        mCreationDate->setText("  " + mEntry->getLastModifiedTime().toLocalTime().toString(mEntry->getDateFormatString()));
        mTextEdit->setHtml(mEntry->getText());
        //setFixedHeight(mTopWidget->height()+mText->height());
    }

    bool CommentItem::search(const QString &string, SearchOptions searchOpts)
    {
        mHeader->handleSearchChanged(string,searchOpts.toInt());
        if (string.isEmpty())
        {
            mTextEdit->setExtraSelections(QList<QTextEdit::ExtraSelection>());
            // reset find marker
            mTextEdit->moveCursor(QTextCursor::Start);
            return false;
        }
        bool found = mHeader->hasMatch();
        QList<QTextEdit::ExtraSelection> extraSelections;

        mTextEdit->moveCursor(QTextCursor::Start);
        QColor color            = QColor(12, 15, 19);
        QColor mBackgroundColor = QColor(255, 255, 0);

        QTextDocument::FindFlags options = QTextDocument::FindFlags();
        options.setFlag(QTextDocument::FindCaseSensitively, searchOpts.isCaseSensitive());
        options.setFlag(QTextDocument::FindWholeWords, searchOpts.isExactMatch());
        qInfo() << "search in commets";
        if(searchOpts.isRegularExpression())
        {
            qInfo() << "is regex";
#if QT_VERSION >= QT_VERSION_CHECK(5,13,0)
            QRegularExpression regExp(string, searchOpts.isCaseSensitive() ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
#else
            QRegExp regExp(string, searchOpts.isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);
#endif

            while (mTextEdit->find(regExp, options))
            {
                found = true; // just return if something is found, position doesnt matter
                QTextEdit::ExtraSelection extra;
                extra.format.setForeground(QBrush(color));
                extra.format.setBackground(mBackgroundColor);
                extra.cursor = mTextEdit->textCursor();
                extraSelections.append(extra);
            }
        }
        else
        {
            while (mTextEdit->find(string, options))
            {

                qInfo() << "found";
                found = true; // just return if something is found, position doesnt matter
                QTextEdit::ExtraSelection extra;
                extra.format.setForeground(QBrush(color));
                extra.format.setBackground(mBackgroundColor);
                extra.cursor = mTextEdit->textCursor();
                extraSelections.append(extra);
            }
        }
        mTextEdit->setExtraSelections(extraSelections);
        return found;
    }

    CommentEntry *CommentItem::getEntry()
    {
        return mEntry;
    }

    void CommentItem::init()
    {
        // extract property information
        style()->unpolish(this);
        style()->polish(this);
        QSize iconSize(20, 20);

        mEntry = nullptr;
        // TODO: replace fixed sizes (put them in stylesheet / compute them?)
        mLayout = new QVBoxLayout(this);
        mLayout->setSpacing(0);
        mLayout->setMargin(0);

        // top part
        mTopWidget = new QWidget(this);
        mTopWidget->setFixedHeight(iconSize.height()+5); // just some spacing
        mTopLayout = new QHBoxLayout(mTopWidget);
        mTopLayout->setSpacing(0);
        mTopLayout->setMargin(0);

        mHeader = new SearchableLabel(this);
        mHeader->setStyleSheet("font-weight: bold;");
        mCreationDate = new QLabel(this);
        mCreationDate->setStyleSheet("font-size: 12px;");
        mModifyButton = new QToolButton(this);
        mModifyButton->setIcon(gui_utility::getStyledSvgIcon(mModifyCommentIconStyle, mModifyCommentIconPath));
        mModifyButton->setIconSize(iconSize);
        mModifyButton->setAutoRaise(true);
        mDeleteButton = new QToolButton(this);
        mDeleteButton->setIcon(gui_utility::getStyledSvgIcon(mDeleteCommentIconStyle, mDeleteCommentIconPath));
        mDeleteButton->setIconSize(iconSize);
        mDeleteButton->setAutoRaise(true);

        //perhaps with alignments (header left, date normal, small spacer, fixed buttons without stretch)?
        mTopLayout->addWidget(mHeader);
        mTopLayout->addWidget(mCreationDate);
        mTopLayout->addWidget(mModifyButton);
        mTopLayout->addWidget(mDeleteButton);
        mTopWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        // actual comment
        mTextEdit = new QTextEdit(this);
        mTextEdit->setReadOnly(true);

        mLayout->addWidget(mTopWidget);
        //mLayout->addWidget(mText);
        mLayout->addWidget(mTextEdit);
        mLayout->addStretch();

        // connections / logic
        connect(mDeleteButton, &QAbstractButton::clicked, this, &CommentItem::handleDeleteButtonTriggered);
        connect(mModifyButton, &QAbstractButton::clicked, this, &CommentItem::handleModifyButtonTriggered);
    }

    void CommentItem::handleDeleteButtonTriggered()
    {
        Q_EMIT delete_requested(this);
    }

    void CommentItem::handleModifyButtonTriggered()
    {
        CommentDialog dialog("Modify Comment", mEntry);
        if(dialog.exec() == QDialog::Accepted)
        {
            mEntry->setHeader(dialog.getHeader());
            mEntry->setText(dialog.getText());
            // relay modification through manager, let comment-widget call this function
            // updateCurrentEntry();
            gCommentManager->relayEntryModified(mEntry);
        }

        dialog.close();
    }
}
