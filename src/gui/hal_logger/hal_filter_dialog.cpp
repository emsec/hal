#include "hal_logger/hal_filter_dialog.h"
#include "hal_logger/hal_filter_item.h"
#include "hal_logger/hal_filter_tab_bar.h"

namespace hal
{
    HalFilterDialog::HalFilterDialog(HalFilterTabBar* caller, QWidget* parent) : QDialog(parent), m_caller(caller), m_button_box(QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
    {
        setWindowTitle("New Filter");
        setSizeGripEnabled(true);
        setMinimumWidth(600);
        setLayout(&m_content_layout);
        m_content_layout.addLayout(&m_form_layout);
        m_name.setPlaceholderText("Filter Name");
        m_keywords.setPlaceholderText("Keywords seperated by commas");
        m_regex.setPlaceholderText("Regular Expression");

        m_form_layout.addRow(&m_name);
        m_form_layout.addRow("<font color=\"DeepPink\">Trace/font>", &m_trace_box);
        m_form_layout.addRow("<font color=\"DeepPink\">Debug</font>", &m_debug_box);
        m_form_layout.addRow("<font color=\"DeepPink\">Info</font>", &m_info_box);
        m_form_layout.addRow("<font color=\"DeepPink\">Warning</font>", &m_warning_box);
        m_form_layout.addRow("<font color=\"DeepPink\">Error</font>", &m_error_box);
        m_form_layout.addRow("<font color=\"DeepPink\">Critical Error</font>", &m_critical_box);
        m_form_layout.addRow("<font color=\"DeepPink\">Default</font>", &m_default_box);
        m_form_layout.addRow(&m_keywords);
        m_form_layout.addRow(&m_regex);

        m_status_message.setStyleSheet("QLabel { background-color: rgba(64, 64, 64, 1);color: rgba(255, 0, 0, 1);border: 1px solid rgba(255, 0, 0, 1)}");
        m_status_message.setText("No Name specified");
        m_status_message.setAlignment(Qt::AlignCenter);
        m_status_message.setMinimumHeight(90);
        m_status_message.hide();
        m_content_layout.addWidget(&m_status_message);
        m_content_layout.addWidget(&m_button_box, Qt::AlignBottom);

        connect(&m_button_box, SIGNAL(accepted()), this, SLOT(verify()));
        connect(&m_button_box, SIGNAL(rejected()), this, SLOT(reject()));

        connect(this, SIGNAL(finished(int)), this, SLOT(reset(int)));
    }

    HalFilterDialog::~HalFilterDialog()
    {
    }

    void HalFilterDialog::verify()
    {
        QString name                        = m_name.text();
        HalFilterItem::rule trace_rule    = m_trace_box.get_data();
        HalFilterItem::rule debug_rule    = m_debug_box.get_data();
        HalFilterItem::rule info_rule     = m_info_box.get_data();
        HalFilterItem::rule warning_rule  = m_warning_box.get_data();
        HalFilterItem::rule error_rule    = m_error_box.get_data();
        HalFilterItem::rule critical_rule = m_critical_box.get_data();
        HalFilterItem::rule default_rule  = m_default_box.get_data();
        QString keywordString               = m_keywords.text();
        QStringList keywordList             = keywordString.split(",", QString::SkipEmptyParts);
        QRegularExpression regex(m_regex.text());

        if (name.isEmpty())
        {
            m_status_message.setText("No Name specified");
            m_status_message.show();
            return;
        }
        else if (!regex.isValid())
        {
            m_status_message.setText("Specified Regular Expression is invalid");
            m_status_message.show();
            return;
        }
        else
        {
            m_status_message.hide();
        }
        HalFilterItem* item = new HalFilterItem(trace_rule, debug_rule, info_rule, warning_rule, error_rule, critical_rule, default_rule, keywordList, regex);
        append_filter_item(name, item);
    }

    void HalFilterDialog::append_filter_item(QString name, HalFilterItem* item)
    {
        m_caller->addNewFilter(name, item);
        accept();
    }

    void HalFilterDialog::reset(int)
    {
        m_name.clear();
        m_trace_box.setCurrentIndex(0);
        m_debug_box.setCurrentIndex(0);
        m_info_box.setCurrentIndex(0);
        m_warning_box.setCurrentIndex(0);
        m_error_box.setCurrentIndex(0);
        m_critical_box.setCurrentIndex(0);
        m_default_box.setCurrentIndex(0);
        m_keywords.clear();
        m_regex.clear();
    }

    HalFilterDialog::filter_combo_box::filter_combo_box(QWidget* parent) : QComboBox(parent)
    {
        addItem("Process", QVariant(0));
        addItem("Show All", QVariant(1));
        addItem("Hide All", QVariant(2));
    }

    HalFilterItem::rule HalFilterDialog::filter_combo_box::get_data()
    {
        switch (currentData().toInt())
        {
            case 0:
                return HalFilterItem::rule::Process;
                break;
            case 1:
                return HalFilterItem::rule::ShowAll;
                break;
            case 2:
                return HalFilterItem::rule::HideAll;
                break;
            default:
                return HalFilterItem::rule::Process;
                break;
        }
    }
}
