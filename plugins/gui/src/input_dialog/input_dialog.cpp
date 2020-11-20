#include "gui/input_dialog/input_dialog.h"

#include "gui/validator/stacked_validator.h"

#include<QDialogButtonBox>

namespace hal
{
    InputDialog::InputDialog(QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f)
    {
        init();
    }

    InputDialog::InputDialog(const QString& window_title, const QString& info_text, const QString& input_text, QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f)
    {
        init();

        setWindowTitle(window_title);
        setInfoText(info_text);
        setInputText(input_text);
    }

    void InputDialog::init()
    {
        mLabelInfoText = new QLabel();
        mLabelWarningText = new QLabel();
        mInputTextEdit = new QLineEdit();

        QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        mOkButton = button_box->button(QDialogButtonBox::Ok);

        mLayout = new QVBoxLayout(this);
        mLayout->addWidget(mLabelInfoText);
        mLayout->addWidget(mInputTextEdit);
        mLayout->addWidget(mLabelWarningText);
        mLayout->addWidget(button_box);


        mLabelInfoText->setStyleSheet("font-weight: bold");
        mLabelInfoText->setContentsMargins(0, 0, 0, 15);

        mOkButton->setDisabled(true);

        mLabelWarningText->setStyleSheet("color: red");
        mLabelWarningText->setAlignment(Qt::AlignHCenter);
        mLabelWarningText->setContentsMargins(0, 0, 0, 3);

        mLayout->setContentsMargins(15, 15, 15, 10);


        mValidator = StackedValidator();

        connect(button_box, &QDialogButtonBox::accepted, this, &InputDialog::handleOkClicked);
        connect(button_box, &QDialogButtonBox::rejected, this, &InputDialog::handleCancelClicked);
        connect(mInputTextEdit, &QLineEdit::textChanged, this, &InputDialog::handleTextChanged);
    }
  
    void InputDialog::setInfoText(const QString& text)
    {
        mLabelInfoText->setText(text);
    }

    void InputDialog::setInputText(const QString& text)
    {
        mInputTextEdit->setText(text.trimmed());
        mInputTextEdit->selectAll();
    }

    void InputDialog::setWarningText(const QString& text)
    {
        mWarningText = text;
    }

    void InputDialog::showWarningText()
    {
        mLabelWarningText->setText(mWarningText);
    }

    void InputDialog::hideWarningText()
    {
        mLabelWarningText->setText("");
    }

    void InputDialog::enableProgression()
    {
        mOkButton->setDisabled(false);
        hideWarningText();
    }

    void InputDialog::disableProgression()
    {
        setWarningText(mValidator.failText());
        mOkButton->setDisabled(true);
        showWarningText();
    }

    void InputDialog::addValidator(Validator* Validator)
    {
        mValidator.addValidator(Validator);
    }

    void InputDialog::removeValidator(Validator* Validator)
    {
        mValidator.removeValidator(Validator);
    }

    void InputDialog::clearValidators()
    {
        mValidator.clearValidators();
    }

    QString InputDialog::textValue() const
    {
        return mInputTextEdit->text().trimmed();
    }

    void InputDialog::handleTextChanged(const QString& text)
    {
        if(!mValidator.validate(text.trimmed()))
            disableProgression();
        else
            enableProgression();
    }

    void InputDialog::handleOkClicked()
    {
        done(QDialog::Accepted);
    }

    void InputDialog::handleCancelClicked()
    {
        done(QDialog::Rejected);
    }
}
