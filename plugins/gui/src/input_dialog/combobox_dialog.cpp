#include "gui/input_dialog/combobox_dialog.h"

#include <QDialogButtonBox>

namespace hal
{
    ComboboxDialog::ComboboxDialog(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
    {
        init();
    }

    ComboboxDialog::ComboboxDialog(const QString windowTitle, const QString infoText, const QStringList entries, const QString defaultSelected, QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
    {
        init();

        setWindowTitle(windowTitle);
        setInfoText(infoText);
        setItems(entries);
        mCombobox->setCurrentText(defaultSelected);
    }

    void ComboboxDialog::setItems(const QStringList entries)
    {
        mCombobox->addItems(entries);
    }

    void ComboboxDialog::setInfoText(const QString text)
    {
        mInfoLabel->setText(text);
    }

    QString ComboboxDialog::textValue() const
    {
        return mCombobox->currentText();
    }

    int ComboboxDialog::selectedIndex() const
    {
        return mCombobox->currentIndex();
    }

    void ComboboxDialog::init()
    {
        mInfoLabel = new QLabel();
        mCombobox = new QComboBox();


        QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        mOkButton = button_box->button(QDialogButtonBox::Ok);

        mLayout = new QVBoxLayout(this);
        mLayout->addWidget(mInfoLabel);
        mLayout->addWidget(mCombobox);
        mLayout->addWidget(button_box);

        mInfoLabel->setStyleSheet("font-weight: bold");
        mInfoLabel->setContentsMargins(0, 0, 0, 15);

        mLayout->setContentsMargins(15,15,15,10);

        connect(button_box, &QDialogButtonBox::accepted, this, &ComboboxDialog::handleOkClicked);
        connect(button_box, &QDialogButtonBox::rejected, this, &ComboboxDialog::handleCancelClicked);
    }

    void ComboboxDialog::handleOkClicked()
    {
        done(QDialog::Accepted);
    }

    void ComboboxDialog::handleCancelClicked()
    {
        done(QDialog::Rejected);

    }

}
