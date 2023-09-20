#include "gui/searchbar/searchoptions_dialog.h"

namespace hal
{
    SearchOptionsDialog::SearchOptionsDialog(QWidget* parent): QDialog(parent)
    {
        setWindowTitle("Search");
        mLayout = new QGridLayout(this);

        mInputBox = new QComboBox();
        mInputBox->setEditable(true);

        mIncrementalSearchBox = new QCheckBox("Incremental search");
        mExactMatchBox = new QCheckBox("Exact match");
        mCaseSensitiveBox = new QCheckBox("Case sensitive");
        mRegExBox = new QCheckBox("Regular expression");

        mColumnLabel = new QLabel();
        mColumnLabel->setText("Search in column");

        mColumnBox = new QComboBox();
        mColumnBox->setEditable(true);

        mSearchBtn = new QPushButton("Search");
        mCloseBtn = new QPushButton("Close");

        mLayout->addWidget(mInputBox, 0, 0, 0, 3);
        mLayout->addWidget(mIncrementalSearchBox, 1, 0);
        mLayout->addWidget(mExactMatchBox, 2, 0);
        mLayout->addWidget(mCaseSensitiveBox, 3, 0);
        mLayout->addWidget(mRegExBox, 4, 0);
        mLayout->addWidget(mColumnLabel, 5, 0);
        mLayout->addWidget(mColumnBox, 5, 1, Qt::AlignLeft);
        mLayout->addWidget(mSearchBtn, 6, 2);
        mLayout->addWidget(mCloseBtn, 6, 3);

        connect(mCloseBtn, &QPushButton::clicked, this, &SearchOptionsDialog::close);

        connect(mIncrementalSearchBox, &QCheckBox::stateChanged, this, &SearchOptionsDialog::optionsChanged);
        connect(mExactMatchBox, &QCheckBox::stateChanged, this, &SearchOptionsDialog::optionsChanged);
        connect(mCaseSensitiveBox, &QCheckBox::stateChanged, this, &SearchOptionsDialog::optionsChanged);
        connect(mRegExBox, &QCheckBox::stateChanged, this, &SearchOptionsDialog::optionsChanged);

    }

    void SearchOptionsDialog::startSearch(QString text, int options)
    {

    }

    void SearchOptionsDialog::optionsChanged()
    {
        searchProxy->updateProxy(mExactMatchBox->isChecked(), mCaseSensitiveBox->isChecked(), mRegExBox->isChecked(), {});
    }
}
