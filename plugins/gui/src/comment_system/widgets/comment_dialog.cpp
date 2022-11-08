#include "gui/comment_system/widgets/comment_dialog.h"
#include "gui/comment_system/comment_entry.h"
#include <QDialogButtonBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QToolBar>
#include <QColorDialog>

namespace hal
{

    CommentDialog::CommentDialog(const QString windowTitle, CommentEntry* entry, QWidget *parent) : QDialog(parent),
        mCommentEntry(entry)
    {
        setWindowTitle(windowTitle);
        init();
        if(entry)
        {
            // fix parsing error and then comment this in!
            //mLastModifiedLabel->setText(mCommentEntry->getLastModifiedTime().toString());
            mHeaderEdit->setText(entry->getHeader());
            mTextEdit->setHtml(entry->getText());
            updateColorActionPixmap(mTextEdit->textColor()); //default text color
        }
    }

    CommentDialog::~CommentDialog()
    {

    }

    void CommentDialog::init()
    {
        setBaseSize(600, 400);

        // header
        mHeaderContainer = new QWidget;
        mHeaderContainerLayout = new QHBoxLayout(mHeaderContainer);
        mHeaderContainerLayout->setContentsMargins(9,20,6,9); // space of layout to surrounding edges (increases container-widget size)
        mHeaderContainer->setStyleSheet("background-color: black");
        mHeaderEdit = new QLineEdit;
        mHeaderEdit->setStyleSheet("background-color: #171e22; color: #A9B7C6;");
        mLastModifiedLabel = new QLabel;
        mLastModifiedLabel->setText(QDateTime::currentDateTime().toString("dd.MM.yy hh:mm")); // default value, is set outside if entry was given in constructor
        //mLastModifiedLabel->setText( mCommentEntry ? mCommentEntry->getLastModifiedTime().toString() : QDateTime::currentDateTime().toString("dd.MM.yy hh:mm"));
        //if(mLastModifiedLabel->text().isEmpty()) mLastModifiedLabel->hide(); (only when an empty string is used instead of current time)

        // perhaps a spacer item with fixed size at the front instead of left spacing/margin
        mHeaderContainerLayout->addWidget(mHeaderEdit);
        mHeaderContainerLayout->addWidget(mLastModifiedLabel);
        //mHeaderContainerLayout->addStretch();

        // toolbar
        // toolbar as qtoolbar
        mToolBar = new QToolBar;
        mBoldAction = mToolBar->addAction("Bold", this, &CommentDialog::boldTriggered); //first arg can be an icon
        mBoldAction->setCheckable(true);
        mBoldAction->setShortcut(Qt::CTRL | Qt::Key_B);
        mToolBar->addSeparator();
        mItalicsAction = mToolBar->addAction("Italics", this, &CommentDialog::italicsTriggered);
        mItalicsAction->setCheckable(true);
        mItalicsAction->setShortcut(Qt::CTRL | Qt::Key_I);
        mToolBar->addSeparator();
        mUnderscoreAction = mToolBar->addAction("Underscore", this, &CommentDialog::underscoreTriggered);
        mUnderscoreAction->setCheckable(true);
        mUnderscoreAction->setShortcut(Qt::CTRL | Qt::Key_U);
        mToolBar->addSeparator(); // returns action...?
        QPixmap pix(16, 16);
        pix.fill(QColor("#A9B7C6")); //put default textcolor here (that is mentioned in the stylesheet)
        mColorAction = mToolBar->addAction(pix, "Colors", this, &CommentDialog::colorTriggered);
        mToolBar->addSeparator();
        mListAction = mToolBar->addAction("List");
        mToolBar->addSeparator();
        mCodeAction = mToolBar->addAction("Code");

        // textedit
        mTextEdit = new QTextEdit;
        QTextCharFormat fmt;
        fmt.setForeground(QColor("#A9B7C6"));//set text color to the default text color
        mTextEdit->mergeCurrentCharFormat(fmt);


        // buttons
        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |  QDialogButtonBox::Cancel);

//        QHBoxLayout* buttonLayout = new QHBoxLayout;
//        mOkButton = new QPushButton("Ok");
//        mCancelButton = new QPushButton("Cancel");
//        buttonLayout->addWidget(mOkButton, 0, Qt::AlignLeft);
//        buttonLayout->addWidget(mCancelButton, 0, Qt::AlignRight);

        // add everything
        mLayout = new QVBoxLayout(this);
        mLayout->addWidget(mHeaderContainer);
        mLayout->addWidget(mToolBar);
        mLayout->addWidget(mTextEdit);
        mLayout->addWidget(buttonBox);
        //mLayout->addLayout(buttonLayout);


        connect(buttonBox, &QDialogButtonBox::accepted, this, &CommentDialog::handleOkClicked);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &CommentDialog::handleCancelClicked);

        connect(mTextEdit, &QTextEdit::currentCharFormatChanged, this, &CommentDialog::handleCurrentCharFormatChanged);
        connect(mTextEdit, &QTextEdit::cursorPositionChanged, this, &CommentDialog::handleCursorPositionChanged);
    }

    void CommentDialog::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
    {
        QTextCursor cursor = mTextEdit->textCursor();
        if(!cursor.hasSelection())
            cursor.select(QTextCursor::WordUnderCursor); // perhaps not necessary? could be quite annoying
        cursor.mergeCharFormat(format);
        mTextEdit->mergeCurrentCharFormat(format);
    }

    void CommentDialog::updateColorActionPixmap(const QColor &c)
    {
        QPixmap pix(16, 16);
        pix.fill(c);
        mColorAction->setIcon(pix);
    }

    void CommentDialog::handleCurrentCharFormatChanged(const QTextCharFormat &format)
    {
        // ugly workaround since the original color is black, but the stylesheet transforms the only the "visuals" (black color is till set as format)
        QColor color = (format.foreground().color() == Qt::black) ? QColor("#A9B7C6") : format.foreground().color();
        //auto color = format.foreground().color();
        updateColorActionPixmap(color);
    }

    void CommentDialog::handleCursorPositionChanged()
    {
        //updateColorActionPixmap(mTextEdit->textCursor().charFormat().foreground().color());
    }

    void CommentDialog::boldTriggered()
    {
        QTextCharFormat fmt;
        fmt.setFontWeight(mBoldAction->isChecked() ? QFont::Bold : QFont::Normal);
        mergeFormatOnWordOrSelection(fmt);
    }

    void CommentDialog::italicsTriggered()
    {
        QTextCharFormat fmt;
        fmt.setFontItalic(mItalicsAction->isChecked());
        mergeFormatOnWordOrSelection(fmt);
    }

    void CommentDialog::underscoreTriggered()
    {
        QTextCharFormat fmt;
        fmt.setFontUnderline(mUnderscoreAction->isChecked());
        mergeFormatOnWordOrSelection(fmt);
    }

    void CommentDialog::colorTriggered()
    {
        //QColorDialog colorDialog(mTextEdit->textColor(), this);
        QColorDialog colorDialog(QColor("#A9B7C6"), this);
        colorDialog.setCustomColor(0, QColor("#A9B7C6")); // the default color textcolor is preserved so that the user can revert to it
        if(colorDialog.exec() == QDialog::Accepted)
        {
            QColor color = colorDialog.selectedColor();
            if(!color.isValid()) return;
            QTextCharFormat fmt;
            fmt.setForeground(color);
            mergeFormatOnWordOrSelection(fmt);
            updateColorActionPixmap(color);
        }
        colorDialog.close(); // otherwise xcb connection error...

//        QColor color = QColorDialog::getColor(mTextEdit->textColor(), this);
//        if(!color.isValid()) return;

//        QTextCharFormat fmt;
//        fmt.setForeground(color);
//        mergeFormatOnWordOrSelection(fmt);
//        updateColorActionPixmap(color);
    }

    void CommentDialog::handleOkClicked()
    {
        done(QDialog::Accepted);
    }

    void CommentDialog::handleCancelClicked()
    {
        done(QDialog::Rejected);
    }

}
