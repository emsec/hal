#include "gui/comment_system/widgets/comment_dialog.h"
#include "gui/comment_system/comment_entry.h"
#include <QDialogButtonBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

namespace hal
{

    CommentDialog::CommentDialog(const QString windowTitle, CommentEntry* entry, QWidget *parent) : QDialog(parent),
        mCommentEntry(entry)
    {
        setWindowTitle(windowTitle);
        init();
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
        mHeaderEdit->setStyleSheet("background-color: white; color: black;");
        mLastModifiedLabel = new QLabel;
        mLastModifiedLabel->setText( mCommentEntry ? mCommentEntry->getLastModifiedTime().toString() : QDateTime::currentDateTime().toString("dd.MM.yy hh:mm"));
        //if(mLastModifiedLabel->text().isEmpty()) mLastModifiedLabel->hide(); (only when an empty string is used instead of current time)

        // perhaps a spacer item with fixed size at the front instead of left spacing/margin
        mHeaderContainerLayout->addWidget(mHeaderEdit);
        mHeaderContainerLayout->addWidget(mLastModifiedLabel);
        //mHeaderContainerLayout->addStretch();

        // toolbar

        // textedit
        mTextEdit = new QTextEdit;

        // buttons
        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |  QDialogButtonBox::Cancel);

        mLayout = new QVBoxLayout(this);
        //mLayout->addStretch();
        mLayout->addWidget(mHeaderContainer);
        mLayout->addWidget(mTextEdit);
        mLayout->addWidget(buttonBox);


        connect(buttonBox, &QDialogButtonBox::accepted, this, &CommentDialog::handleOkClicked);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &CommentDialog::handleCancelClicked);
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
