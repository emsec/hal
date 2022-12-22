#include "gui/comment_system/widgets/comment_color_picker.h"
#include <QDebug>
#include <QRadioButton>
#include <QDialogButtonBox>

namespace hal
{

    CommentColorPicker::CommentColorPicker(QWidget* parent) : QDialog(parent),
        mRedColor(QColor(Qt::red)), mGreenColor(QColor(Qt::green)), mYellowColor(QColor(Qt::yellow))
    {
        mDefaultColor = QColor("#A9B7C6");

        mLayout = new QVBoxLayout(this);
        QPixmap pix(16,16);

        mRedButton = new QRadioButton("Red", this);
        pix.fill(mRedColor);
        mRedButton->setIcon(pix);
        mRedButton->setChecked(true);
        //mRedButton->setStyleSheet("background-color: blue;");

        mYellowButton = new QRadioButton("Yellow", this);
        pix.fill(mYellowColor);
        mYellowButton->setIcon(pix);

        mGreenButton = new QRadioButton("Green", this);
        pix.fill(mGreenColor);
        mGreenButton->setIcon(pix);


        mDefaultButton = new QRadioButton("Default", this);
        pix.fill(mDefaultColor); // stylesheet dependent
        mDefaultButton->setIcon(pix);

        QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

        mLayout->addWidget(mRedButton);
        mLayout->addWidget(mYellowButton);
        mLayout->addWidget(mGreenButton);
        mLayout->addWidget(mDefaultButton);
        mLayout->addStretch();
        mLayout->addWidget(btnBox);

        connect(btnBox, &QDialogButtonBox::accepted, this, &CommentColorPicker::handleOkClicked);
        connect(btnBox, &QDialogButtonBox::rejected, this, &CommentColorPicker::handleCancelClicked);

        // hacky as hell
//        show();
//        setFixedSize(size());
//        hide();

    }

    void CommentColorPicker::setInitColor(Color color)
    {
        switch (color)
        {
            case Color::Red: mRedButton->setChecked(true); break;
            case Color::Yellow: mYellowButton->setChecked(true); break;
            case Color::Green: mGreenButton->setChecked(true); break;
            default: mDefaultButton->setChecked(true);
        }
    }

    QColor CommentColorPicker::getSelectedColor()
    {
        if(mRedButton->isChecked())
            return mRedColor;
        if(mYellowButton->isChecked())
            return mYellowColor;
        if(mGreenButton->isChecked())
            return mGreenColor;
        if(mDefaultButton->isChecked())
            return mDefaultColor;
        Q_ASSERT(1==0); // all legal cases covered by if-statements above
        return QColor();
    }

    void CommentColorPicker::handleOkClicked()
    {
        done(QDialog::Accepted);
    }

    void CommentColorPicker::handleCancelClicked()
    {
        done(QDialog::Rejected);
    }


}
