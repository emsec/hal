#include "gui/comment_system/widgets/comment_dialog.h"
#include "gui/comment_system/comment_entry.h"
#include "gui/comment_system/widgets/comment_color_picker.h"
#include <QDialogButtonBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QToolBar>
#include <QColorDialog>

#include <QTextList>
#include <QMenuBar>
#include <QMenu>
#include <QDebug>

#include <QFontDatabase>
#include <QStyle>

namespace hal
{

    CommentDialog::CommentDialog(const QString windowTitle, CommentEntry* entry, QWidget *parent) : QDialog(parent),
        mCommentEntry(entry)
    {
        setWindowTitle(windowTitle);
        init();
        // should be extracted from stylesheet, still doesnt work...
        mDefaultFont = QFont("Iosevka");
        mDefaultFont.setPixelSize(14);

        if(entry)
        {
            // fix parsing error and then comment this in!
            mLastModifiedLabel->setText(mCommentEntry->getLastModifiedTime().toString(mCommentEntry->getDateFormatString()));
            mHeaderEdit->setText(entry->getHeader());
            mTextEdit->setHtml(entry->getText());
            updateColorActionPixmap(mDefaultColor); //default text color
        }
    }

    CommentDialog::~CommentDialog()
    {

    }

    QString CommentDialog::getHeader()
    {
        return mHeaderEdit->text();
    }

    QString CommentDialog::getText()
    {
        return mTextEdit->toHtml();
    }

    void CommentDialog::init()
    {
        setBaseSize(600, 400);

        QStyle* s = style();
        s->unpolish(this);
        s->polish(this);

        // header
        mHeaderContainer = new QWidget;
        mHeaderContainerLayout = new QHBoxLayout(mHeaderContainer);
        mHeaderContainerLayout->setContentsMargins(9,20,6,9); // space of layout to surrounding edges (increases container-widget size)
        //mHeaderContainer->setStyleSheet("background-color: black");
        mHeaderContainer->setObjectName("header-container");
        mHeaderEdit = new QLineEdit;
        //mHeaderEdit->setStyleSheet("background-color: #171e22; color: #A9B7C6;");
        mLastModifiedLabel = new QLabel;

        // perhaps a spacer item with fixed size at the front instead of left spacing/margin
        mHeaderContainerLayout->addWidget(mHeaderEdit);
        mHeaderContainerLayout->addWidget(mLastModifiedLabel);

        // toolbar
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
        pix.fill(QColor(mDefaultColor));
        mColorAction = mToolBar->addAction(pix, "Colors", this, &CommentDialog::colorTriggered);
        mToolBar->addSeparator();
        mListAction = mToolBar->addAction("List", this, &CommentDialog::bulletListTriggered);
        mListAction->setCheckable(true);
        mToolBar->addSeparator();
        mCodeAction = mToolBar->addAction("Code", this, &CommentDialog::codeTriggered);
        mCodeAction->setCheckable(true);

        // textedit
        mTextEdit = new QTextEdit;
//        QTextCharFormat fmt;
//        fmt.setForeground(QColor(mDefaultColor));//set text color to the default text color
//        mTextEdit->mergeCurrentCharFormat(fmt);

        // buttons
        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |  QDialogButtonBox::Cancel, this);

        // add everything
        mLayout = new QVBoxLayout(this);
        mLayout->addWidget(mHeaderContainer);
        mLayout->addWidget(mToolBar);
        mLayout->addWidget(mTextEdit);
        mLayout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, this, &CommentDialog::handleOkClicked);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &CommentDialog::handleCancelClicked);

        connect(mTextEdit, &QTextEdit::currentCharFormatChanged, this, &CommentDialog::handleCurrentCharFormatChanged);
        connect(mTextEdit, &QTextEdit::cursorPositionChanged, this, &CommentDialog::handleCursorPositionChanged);

        s = style();
        s->unpolish(this);
        s->polish(this);
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

    void CommentDialog::handleColorSelected()
    {
        auto act = dynamic_cast<QAction*>(sender());
        if(!act) return;

        auto color = act->data().value<QColor>();
        if(!color.isValid()) return;

        QTextCharFormat fmt;
        fmt.setForeground(color);
        mergeFormatOnWordOrSelection(fmt);
        updateColorActionPixmap(color);
    }

    void CommentDialog::handleCurrentCharFormatChanged(const QTextCharFormat &format)
    {
        // ugly workaround since the original color is black, but the stylesheet transforms only the "visuals" (black color is still set as format)
        QColor color = (format.foreground().color() == Qt::black) ? mDefaultColor : format.foreground().color();
        mBoldAction->setChecked(format.font().bold());
        mItalicsAction->setChecked(format.font().italic());
        mUnderscoreAction->setChecked(format.font().underline());
        mCodeAction->setChecked(format.background().color() == mTextEdit->palette().color(QPalette::AlternateBase)); // put these strings in vars...
        updateColorActionPixmap(color);
    }

    void CommentDialog::handleCursorPositionChanged()
    {
        //updateColorActionPixmap(mTextEdit->textCursor().charFormat().foreground().color());
        mListAction->setChecked(mTextEdit->textCursor().currentList()); // yes this works, isnt c++ fun
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
        //QColor color(Qt::red);
        QColor color(mRedColor);
        QMenu* men = new QMenu(this);
        QPixmap map(16, 16);
        map.fill(color);
        auto act = men->addAction(map, "Red", this, &CommentDialog::handleColorSelected);
        act->setData(color);

        color = QColor(mGreenColor);
        map.fill(color);
        act = men->addAction(map, "Green", this, &CommentDialog::handleColorSelected);
        act->setData(color);

        color = QColor(mYellowColor);
        map.fill(color);
        act = men->addAction(map, "Yellow", this, &CommentDialog::handleColorSelected);
        act->setData(color);

        color = QColor(mDefaultColor); // out of stylesheet
        map.fill(color);
        act = men->addAction(map, "Default", this, &CommentDialog::handleColorSelected);
        act->setData(color);

        QRect geo = mToolBar->actionGeometry(mColorAction);
        QPoint global = mToolBar->mapToGlobal(QPoint(geo.x(), geo.y()));
        men->move(global.x(), global.y() + geo.height());
        men->exec();
    }

    void CommentDialog::bulletListTriggered()
    {
        QTextCursor cursor = mTextEdit->textCursor();
        if(cursor.currentList())
        {
            QTextBlock block = cursor.block();
            cursor.currentList()->remove(block);
            QTextBlockFormat blockFmt = cursor.blockFormat();
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }
        else
        {
            QTextListFormat::Style style = QTextListFormat::ListDisc;
            QTextBlockFormat::MarkerType marker = QTextBlockFormat::MarkerType::NoMarker;

            cursor.beginEditBlock();

            QTextBlockFormat blockFmt = cursor.blockFormat();
            blockFmt.setMarker(marker);
            cursor.setBlockFormat(blockFmt);
            QTextListFormat listFmt;
            if(cursor.currentList())
                listFmt = cursor.currentList()->format();
            else
            {
                listFmt.setIndent(blockFmt.indent()+1);
                blockFmt.setIndent(0);
                cursor.setBlockFormat(blockFmt);
            }
            listFmt.setStyle(style);
            cursor.createList(listFmt);
            cursor.endEditBlock();
        }
    }

    void CommentDialog::codeTriggered()
    {
        QTextCharFormat fmt;
        //mTextEdit->palette().color(QPalette::Background)
        //fmt.setBackground(mCodeAction->isChecked() ? QColor("#334652") : mTextEdit->palette().color(QPalette::Background)); //dependent on background
        fmt.setBackground(mCodeAction->isChecked() ? mTextEdit->palette().color(QPalette::AlternateBase) : mTextEdit->palette().color(QPalette::Background));
        fmt.setFont(mCodeAction->isChecked() ? QFontDatabase::systemFont(QFontDatabase::FixedFont) : mDefaultFont);
        mergeFormatOnWordOrSelection(fmt);
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
