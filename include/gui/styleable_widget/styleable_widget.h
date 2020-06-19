#pragma once

#include <QWidget>
namespace hal{
class styleable_widget : public QWidget
{
    Q_OBJECT

public:
    explicit styleable_widget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event);
};
}
