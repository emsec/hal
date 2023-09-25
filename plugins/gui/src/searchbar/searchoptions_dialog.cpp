#include "gui/searchbar/searchoptions_dialog.h"
#include <QDebug>

namespace hal
{
    SearchOptionsDialog::SearchOptionsDialog(QWidget* parent): QDialog(parent)
    {
        setWindowTitle("Search");
        searchProxy = new SearchProxyModel();
        //TODO fix layout size to prevent overlapping
        mLayout = new QGridLayout(this);
        mLayout->setRowMinimumHeight(0,35);
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

        QString searchString;

        mLayout->addWidget(mInputBox, 0, 0, 0, 3, Qt::AlignTop);
        mLayout->addWidget(mIncrementalSearchBox, 1, 0);
        mLayout->addWidget(mExactMatchBox, 2, 0);
        mLayout->addWidget(mCaseSensitiveBox, 3, 0);
        mLayout->addWidget(mRegExBox, 4, 0);
        mLayout->addWidget(mColumnLabel, 5, 0);
        mLayout->addWidget(mColumnBox, 5, 1, Qt::AlignLeft);
        mLayout->addWidget(mSearchBtn, 6, 2);
        mLayout->addWidget(mCloseBtn, 6, 3);

        connect(mCloseBtn, &QPushButton::clicked, this, &SearchOptionsDialog::close);

        //TODO get corresponding proxy
        connect(mInputBox, &QComboBox::currentTextChanged, this, &SearchOptionsDialog::textEdited);
        connect(mSearchBtn, &QPushButton::clicked, this, &SearchOptionsDialog::emitStartSearch);
        connect(this, SIGNAL(emitOptions(QString, int)), searchProxy, SLOT(startSearch(QString, int)));

        connect(mIncrementalSearchBox, &QCheckBox::stateChanged, this, &SearchOptionsDialog::optionsChanged);
        connect(mExactMatchBox, &QCheckBox::stateChanged, this, &SearchOptionsDialog::optionsChanged);
        connect(mCaseSensitiveBox, &QCheckBox::stateChanged, this, &SearchOptionsDialog::optionsChanged);
        connect(mRegExBox, &QCheckBox::stateChanged, this, &SearchOptionsDialog::optionsChanged);

    }

    void SearchOptionsDialog::textEdited(QString text)
    {
        //check if incremental search is enabled and start and min 3 symbols
        if(mIncrementalSearchBox->isChecked() && text.length() >= 3){
            emitOptions();
        }
    }

    void SearchOptionsDialog::emitStartSearch()
    {
        emitOptions();
    }

    void SearchOptionsDialog::optionsChanged()
    {
       // searchProxy->startSearch(12, "");
       //searchProxy->updateProxy(mExactMatchBox->isChecked(), mCaseSensitiveBox->isChecked(), mRegExBox->isChecked(), {}, searchText);
    }

    void SearchOptionsDialog::emitOptions(){
        searchText = mInputBox->currentText();
        int options = SearchOptions::toInt(mExactMatchBox->isChecked(), mCaseSensitiveBox->isChecked(), mRegExBox->isChecked(), {});
        qInfo() << "Emit search with string: " << searchText << " and options: " << options;

        Q_EMIT emitOptions(searchText, options);
    }

    SearchOptions SearchOptionsDialog::getOptions() const
    {
        SearchOptions retval;
        //  TODO :: retval.setOptions()
        return retval;
    }

}
