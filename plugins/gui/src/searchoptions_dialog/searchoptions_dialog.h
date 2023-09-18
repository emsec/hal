#pragma once

#include <QDialog>
#include <QGridLayout>

namespace hal
{
    class SearchOptionsDialog : public QDialog
    {
        Q_OBJECT
    public:
        /**
         * Constructor. Initializes the Dialog.
         *
         * @param parent - The parent widget.
         */
        explicit SearchOptionsDialog(QWidget *parent = nullptr);

    private:
        QGridLayout* mLayout;
    };
}
