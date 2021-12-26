#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>

class QStringList;

namespace hal
{
    class ComboboxDialog : public QDialog
    {
    public:
        ComboboxDialog(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
        ComboboxDialog(const QString windowTitle, const QString infoText, const QStringList entries, const QString defaultSelected = "", QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());


        void setItems(const QStringList entries);
        void setInfoText(const QString text);

        QString textValue() const;
        int selectedIndex() const;

    private:
        QVBoxLayout* mLayout;
        QLabel* mInfoLabel;
        QComboBox* mCombobox;
        QPushButton* mOkButton;

        void init();

        void handleOkClicked();
        void handleCancelClicked();

    };

}
