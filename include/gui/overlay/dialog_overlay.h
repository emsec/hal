#pragma once

#include "overlay/overlay.h"

class QVBoxLayout;

namespace hal{

class dialog_overlay : public overlay
{
    Q_OBJECT

public:
    dialog_overlay(QWidget* parent = nullptr);

    void set_widget(QWidget* widget);

//protected:
//    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

private:
    QVBoxLayout* m_layout;
    QWidget* m_widget;
};
}
