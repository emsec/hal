#include "gui/searchbar/searchoptions_dialog.h"
#include "gui/searchbar/searchcolumndialog.h"
#include <QDebug>
#include <QListWidget>

namespace hal
{
    SearchOptionsDialog::SearchOptionsDialog(QWidget* parent): QDialog(parent)
    {
        setWindowTitle("Search");
        //TODO fix layout size to prevent overlapping
        mLayout = new QGridLayout(this);
        mLayout->setRowMinimumHeight(0,35);

        //Searchstring inputbox
        mInputBox = new QComboBox(this);
        mInputBox->setEditable(true);
        mLineEdit = mInputBox->lineEdit();

        //Incremental search widgets
        mIncrementalSearchBox = new QCheckBox("Incremental search", this);
        mIncrementalSearchBox->setChecked(true);
        mSpinBoxLabel = new QLabel("start at: ", this);
        mSpinBox = new QSpinBox(this);
        mSpinBox->setMinimum(1);
        mSpinBox->setMaximum(50);
        mSpinBox->setValue(3);
        mSpinBox->setSuffix(" chars");

        //Exact match widget
        mExactMatchBox = new QCheckBox("Exact match", this);

        //Case sensitive widget
        mCaseSensitiveBox = new QCheckBox("Case sensitive", this);

        //RegEx widget
        mRegExBox = new QCheckBox("Regular expression", this);

        //Column widgets
        /*mColumnLabel = new QLabel(this);
        mColumnLabel->setText("Search in:");*/

        /*mListWidget = new QListWidget(this);
        mListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        mListWidget->setMaximumHeight(150);*/


        //Pushbuttons
        mSearchBtn = new QPushButton("Search", this);
        mCloseBtn = new QPushButton("Close", this);
        mSelectColumnsBtn = new QPushButton("Search in columns", this);

        //Connect widgets with layout
        mLayout->addWidget(mInputBox, 0, 0, 0, 3, Qt::AlignTop);
        //mLayout->addWidget(mColumnLabel, 1, 2);
        mLayout->addWidget(mIncrementalSearchBox, 1, 0, 1, 2);
        //mLayout->addWidget(mListWidget, 2, 2, 5, Qt::AlignLeft);
        mLayout->addWidget(mSpinBoxLabel, 2, 0, Qt::AlignRight);
        mLayout->addWidget(mSpinBox, 2, 1);
        mLayout->addWidget(mExactMatchBox, 3, 0, 1, 2);
        mLayout->addWidget(mCaseSensitiveBox, 4, 0, 1, 2);
        mLayout->addWidget(mRegExBox, 5, 0, 1, 2);
        mLayout->addWidget(mSelectColumnsBtn, 6, 0);
        mLayout->addWidget(mSearchBtn, 7, 0);
        mLayout->addWidget(mCloseBtn, 7, 1);
        mLayout->setHorizontalSpacing(20);

        connect(mCloseBtn, &QPushButton::clicked, this, &SearchOptionsDialog::close);
        connect(mSearchBtn, &QPushButton::clicked, this, &SearchOptionsDialog::emitStartSearch);
        connect(mSelectColumnsBtn, &QPushButton::clicked, this, &SearchOptionsDialog::selectColumns);
        connect(mIncrementalSearchBox, &QCheckBox::stateChanged, this, &SearchOptionsDialog::incrementalSearchToggled);
        //connect(mListWidget, &QListWidget::itemSelectionChanged, this, &SearchOptionsDialog::testWidget);

        //TODO maybe delete this because edit triggers also the mSearchBtn signal as if it was clicked. Currently the emit search is emited twice while pressing Enter
        // discuss with Joern
        connect(mLineEdit, &QLineEdit::returnPressed, this, &SearchOptionsDialog::emitStartSearch);


        // de-/ activate spinBox widgets based on IncCheckBox
        this->incrementalSearchToggled(mIncrementalSearchBox->isChecked());
    }

    //TODO can be deleted later on
   /* void SearchOptionsDialog::testWidget()
    {
        auto list = mListWidget->selectedItems();

        for(auto elem : list){
            qInfo() << elem->text();
        }
    }*/

    void SearchOptionsDialog::emitStartSearch()
    {
        qInfo() << "emitStartSearch from searchOptionsDialog";
        emitOptions();
    }


    void SearchOptionsDialog::emitOptions()
    {
        mSearchText = mInputBox->currentText();
        int options = SearchOptions::toInt(mExactMatchBox->isChecked(), mCaseSensitiveBox->isChecked(), mRegExBox->isChecked(), mSelectedColumns); //TO-DO: fill the columns
        qInfo() << "Emit search with string: " << mSearchText << " and options: " << options;

        Q_EMIT emitOptions(mSearchText, options);
        Q_EMIT accept();
    }

    SearchOptions* SearchOptionsDialog::getOptions() const
    {
        SearchOptions* retval = new SearchOptions();

        //get index of each selected column
        /*QList<int> selectedItems = {};
        for(auto elem : mListWidget->selectedItems()){
            int index = mListWidget->row(elem);
            if(index > 0)
                selectedItems.append(index - 1);
            else{
                selectedItems = {};
                break;
            }
        }*/
        retval->setOptions(mExactMatchBox->isChecked(), mCaseSensitiveBox->isChecked(), mRegExBox->isChecked(), mSelectedColumns); //TO-DO: fill the columns
        return retval;
    }

    void SearchOptionsDialog::setOptions(SearchOptions* opts, QString searchString, QList<QString> columnNames, bool incSearch, int minIncSearch)
    {
        //set the parameters of the Dialog to previous ones or do default
        mLineEdit->setText(searchString);

        QStringList a = {};
        mExactMatchBox->setChecked(opts->isExactMatch());
        mCaseSensitiveBox->setChecked(opts->isCaseSensitive());
        mRegExBox->setChecked(opts->isRegularExpression());
        mIncrementalSearchBox->setChecked(incSearch);

        if(!columnNames.isEmpty())
        {
            for(QString name : columnNames)
                mColumnNames.append(name);
        }
        //add the given column names into the widget if there are any, if not then hide the widget
        /*if(!columnNames.isEmpty()){
            mListWidget->addItem("All columns");
            for (auto elem : columnNames)
            {
                mListWidget->addItem(elem);
            }
        }else{
            mListWidget->hide();
            mColumnLabel->hide();
        }*/
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

    void SearchOptionsDialog::selectColumns()
    {

        hal::SearchColumnDialog scd(mColumnNames, mSelectedColumns);
        if (scd.exec() == QDialog::Accepted)
        {

            mSelectColumnsBtn->setText(scd.selectedColumnNames());
            mSelectedColumns = scd.selectedColumns();
        }
    }

}
