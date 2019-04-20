#include "graph_layouter/input_dialog.h"
#include "netlist/gate.h"
#include "netlist/net.h"
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>

input_dialog::input_dialog(QWidget* parent, QString query, QString init_str) : QDialog(parent)
{
    init(query, init_str);
}

QString input_dialog::get_text() const
{
    return edit_name->text();
}

void input_dialog::init(QString query, QString init)
{
    layout        = new QGridLayout(this);
    label_name    = new QLabel(query);
    edit_name     = new QLineEdit();
    save_button   = new QPushButton("Ok");
    cancel_button = new QPushButton("Cancel");
    cancel_button->setMaximumWidth(cancel_button->width());
    edit_name->setText(init);

    edit_name->setFixedWidth(400);
    layout->addWidget(label_name, 0, 0, Qt::AlignLeft);
    layout->addWidget(edit_name, 0, 1, Qt::AlignLeft);
    layout->addWidget(save_button, 1, 0, Qt::AlignLeft);
    layout->addWidget(cancel_button, 1, 1, Qt::AlignLeft);
    this->setLayout(layout);

    this->move(QApplication::desktop()->screen()->rect().center() - this->rect().center());
    this->setWindowFlags(Qt::Widget | Qt::MSWindowsFixedSizeDialogHint);

    QObject::connect(save_button, &QPushButton::clicked, this, &input_dialog::accept);
    QObject::connect(cancel_button, &QPushButton::clicked, this, &input_dialog::reject);
    QObject::connect(edit_name, &QLineEdit::returnPressed, this, &input_dialog::accept);
}
