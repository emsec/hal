#include "gui/searchbar/searchoptions_dialog.h"
#include <QDebug>

namespace hal
{
    SearchOptionsDialog::SearchOptionsDialog(QWidget* parent): QDialog(parent)
    {
        setWindowTitle("Search");
        //TODO fix layout size to prevent overlapping
        mLayout = new QGridLayout(this);
        mLayout->setRowMinimumHeight(0,35);

        //Searchstring inputbox
        mInputBox = new QComboBox();
        mInputBox->setEditable(true);
        mLineEdit = mInputBox->lineEdit();

        //Incremental search widgets
        mIncrementalSearchBox = new QCheckBox("Incremental search");
        mIncrementalSearchBox->setChecked(true);
        mSpinBoxLabel = new QLabel("start at: ");
        mSpinBox = new QSpinBox();
        mSpinBox->setMinimum(1);
        mSpinBox->setMaximum(50);
        mSpinBox->setValue(3);
        mSpinBox->setSuffix(" chars");

        //Exact match widget
        mExactMatchBox = new QCheckBox("Exact match");

        //Case sensitive widget
        mCaseSensitiveBox = new QCheckBox("Case sensitive");

        //RegEx widget
        mRegExBox = new QCheckBox("Regular expression");

        //Column widgets
        mColumnLabel = new QLabel();
        mColumnLabel->setText("Search in column");
        mColumnBox = new QComboBox();
        mColumnBox->setEditable(true);

        //Pushbuttons
        mSearchBtn = new QPushButton("Search");
        mCloseBtn = new QPushButton("Close");

        //Connect widgets with layout
        mLayout->addWidget(mInputBox, 0, 0, 0, 3, Qt::AlignTop);
        mLayout->addWidget(mIncrementalSearchBox, 1, 0);
        mLayout->addWidget(mSpinBoxLabel, 1, 1, Qt::AlignRight);
        mLayout->addWidget(mSpinBox, 1, 2, Qt::AlignLeft);
        mLayout->addWidget(mExactMatchBox, 2, 0);
        mLayout->addWidget(mCaseSensitiveBox, 3, 0);
        mLayout->addWidget(mRegExBox, 4, 0);
        mLayout->addWidget(mColumnLabel, 5, 0);
        mLayout->addWidget(mColumnBox, 5, 1, Qt::AlignLeft);
        mLayout->addWidget(mSearchBtn, 6, 1);
        mLayout->addWidget(mCloseBtn, 6, 2);

        connect(mCloseBtn, &QPushButton::clicked, this, &SearchOptionsDialog::close);
        connect(mSearchBtn, &QPushButton::clicked, this, &SearchOptionsDialog::emitStartSearch);
        connect(mIncrementalSearchBox, &QCheckBox::stateChanged, this, &SearchOptionsDialog::incrementalSearchToggled);

        //TODO maybe delete this because edit triggers also the mSearchBtn signal as if it was clicked. Currently the emit search is emited twice while pressing Enter
        // discuss with Joern
        connect(mLineEdit, &QLineEdit::returnPressed, this, &SearchOptionsDialog::emitStartSearch);


        // de-/ activate spinBox widgets based on IncCheckBox
        this->incrementalSearchToggled(mIncrementalSearchBox->isChecked());
    }

    void SearchOptionsDialog::emitStartSearch()
    {
        qInfo() << "emitStartSearch from searchOptionsDialog";
        emitOptions();
    }


    void SearchOptionsDialog::emitOptions()
    {
        mSearchText = mInputBox->currentText();
        int options = SearchOptions::toInt(mExactMatchBox->isChecked(), mCaseSensitiveBox->isChecked(), mRegExBox->isChecked(), {}); //TO-DO: fill the columns
        qInfo() << "Emit search with string: " << mSearchText << " and options: " << options;

        Q_EMIT emitOptions(mSearchText, options);
        Q_EMIT accept();
    }

    SearchOptions* SearchOptionsDialog::getOptions() const
    {
        SearchOptions* retval = new SearchOptions();
        retval->setOptions(mExactMatchBox->isChecked(), mCaseSensitiveBox->isChecked(), mRegExBox->isChecked(), {}); //TO-DO: fill the columns
        return retval;
    }

    void SearchOptionsDialog::setOptions(SearchOptions* opts, QString searchString, bool incSearch, int minIncSearch) const
    {
        //set the parameters of the Dialog to previous ones or do default
        mLineEdit->setText(searchString);
        mExactMatchBox->setChecked(opts->isExactMatch());
        mCaseSensitiveBox->setChecked(opts->isCaseSensitive());
        mRegExBox->setChecked(opts->isRegularExpression());
        mIncrementalSearchBox->setChecked(incSearch);

        mSpinBox->setValue(minIncSearch);
    }

    QString SearchOptionsDialog::getText() const
    {
        return mSearchText;
    }


    int SearchOptionsDialog::getMinIncSearchValue()
    {
        return mSpinBox->value();
    }

    bool SearchOptionsDialog::getIncrementalSearch()
    {
        return mIncrementalSearchBox->isChecked();
    }

    void SearchOptionsDialog::incrementalSearchToggled(int state)
    {

        mSpinBoxLabel->setDisabled(!state);
        mSpinBox->setDisabled(!state);

    }

}
