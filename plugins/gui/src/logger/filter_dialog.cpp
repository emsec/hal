#include "gui/logger/filter_dialog.h"
#include "gui/logger/filter_item.h"
#include "gui/logger/filter_tab_bar.h"

namespace hal
{
    FilterDialog::FilterDialog(FilterTabBar* caller, QWidget* parent) : QDialog(parent), mCaller(caller), mButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
    {
        setWindowTitle("New Filter");
        setSizeGripEnabled(true);
        setMinimumWidth(600);
        setLayout(&mContentLayout);
        mContentLayout.addLayout(&mFormLayout);
        mName.setPlaceholderText("Filter Name");
        mKeywords.setPlaceholderText("Keywords seperated by commas");
        mRegex.setPlaceholderText("Regular Expression");

        mFormLayout.addRow(&mName);
        mFormLayout.addRow("<font color=\"DeepPink\">Trace/font>", &mTraceBox);
        mFormLayout.addRow("<font color=\"DeepPink\">Debug</font>", &mDebugBox);
        mFormLayout.addRow("<font color=\"DeepPink\">Info</font>", &mInfoBox);
        mFormLayout.addRow("<font color=\"DeepPink\">Warning</font>", &mWarningBox);
        mFormLayout.addRow("<font color=\"DeepPink\">Error</font>", &mErrorBox);
        mFormLayout.addRow("<font color=\"DeepPink\">Critical Error</font>", &mCriticalBox);
        mFormLayout.addRow("<font color=\"DeepPink\">Default</font>", &mDefaultBox);
        mFormLayout.addRow(&mKeywords);
        mFormLayout.addRow(&mRegex);

        mStatusMessage.setStyleSheet("QLabel { background-color: rgba(64, 64, 64, 1);color: rgba(255, 0, 0, 1);border: 1px solid rgba(255, 0, 0, 1)}");
        mStatusMessage.setText("No Name specified");
        mStatusMessage.setAlignment(Qt::AlignCenter);
        mStatusMessage.setMinimumHeight(90);
        mStatusMessage.hide();
        mContentLayout.addWidget(&mStatusMessage);
        mContentLayout.addWidget(&mButtonBox, Qt::AlignBottom);

        connect(&mButtonBox, SIGNAL(accepted()), this, SLOT(verify()));
        connect(&mButtonBox, SIGNAL(rejected()), this, SLOT(reject()));

        connect(this, SIGNAL(finished(int)), this, SLOT(reset(int)));
    }

    FilterDialog::~FilterDialog()
    {
    }

    void FilterDialog::verify()
    {
        QString name                        = mName.text();
        FilterItem::Rule trace_rule    = mTraceBox.get_data();
        FilterItem::Rule debug_rule    = mDebugBox.get_data();
        FilterItem::Rule info_rule     = mInfoBox.get_data();
        FilterItem::Rule warning_rule  = mWarningBox.get_data();
        FilterItem::Rule error_rule    = mErrorBox.get_data();
        FilterItem::Rule critical_rule = mCriticalBox.get_data();
        FilterItem::Rule default_rule  = mDefaultBox.get_data();
        QString keywordString               = mKeywords.text();
        QStringList keywordList             = keywordString.split(",", QString::SkipEmptyParts);
        QRegularExpression regex(mRegex.text());

        if (name.isEmpty())
        {
            mStatusMessage.setText("No Name specified");
            mStatusMessage.show();
            return;
        }
        else if (!regex.isValid())
        {
            mStatusMessage.setText("Specified Regular Expression is invalid");
            mStatusMessage.show();
            return;
        }
        else
        {
            mStatusMessage.hide();
        }
        FilterItem* item = new FilterItem(trace_rule, debug_rule, info_rule, warning_rule, error_rule, critical_rule, default_rule, keywordList, regex);
        appendFilterItem(name, item);
    }

    void FilterDialog::appendFilterItem(QString name, FilterItem* item)
    {
        mCaller->addNewFilter(name, item);
        accept();
    }

    void FilterDialog::reset(int)
    {
        mName.clear();
        mTraceBox.setCurrentIndex(0);
        mDebugBox.setCurrentIndex(0);
        mInfoBox.setCurrentIndex(0);
        mWarningBox.setCurrentIndex(0);
        mErrorBox.setCurrentIndex(0);
        mCriticalBox.setCurrentIndex(0);
        mDefaultBox.setCurrentIndex(0);
        mKeywords.clear();
        mRegex.clear();
    }

    FilterDialog::FilterComboBox::FilterComboBox(QWidget* parent) : QComboBox(parent)
    {
        addItem("Process", QVariant(0));
        addItem("Show All", QVariant(1));
        addItem("Hide All", QVariant(2));
    }

    FilterItem::Rule FilterDialog::FilterComboBox::get_data()
    {
        switch (currentData().toInt())
        {
            case 0:
                return FilterItem::Rule::Process;
                break;
            case 1:
                return FilterItem::Rule::ShowAll;
                break;
            case 2:
                return FilterItem::Rule::HideAll;
                break;
            default:
                return FilterItem::Rule::Process;
                break;
        }
    }
}
