#ifndef KEYBIND_EDIT_H
#define KEYBIND_EDIT_H

#include "validator/stacked_validator.h"

#include <QKeySequenceEdit>
#include <QEvent>

class keybind_edit : public QKeySequenceEdit
{
Q_OBJECT
Q_PROPERTY(bool validated READ validated WRITE set_validated)
public:
    keybind_edit(QWidget* parent = nullptr);
    void add_validator(validator* v);
    void set_validated(bool validated);
    bool validated();
    void revalidate();

Q_SIGNALS:
    void edit_rejected();

protected:
    bool event(QEvent* e) Q_DECL_OVERRIDE;

private:
    stacked_validator m_validator;
    QKeySequence m_old_sequence;
    bool m_validated = true;
};

#endif // KEYBIND_EDIT_H

