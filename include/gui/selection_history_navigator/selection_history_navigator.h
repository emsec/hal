#ifndef SELECTION_HISTORY_NAVIGATOR_H
#define SELECTION_HISTORY_NAVIGATOR_H

#include "def.h"

#include <QObject>
#include <list>

class selection_history_navigator : public QObject
{
    Q_OBJECT

public:
    selection_history_navigator(QObject* parent = 0, unsigned int max_history_size = 20);
    ~selection_history_navigator();
    
    void navigate_to_prev_gate();
    void navigate_to_next_gate();

    void set_max_history_size(unsigned int max_size);
    u32 get_max_history_size();

Q_SIGNALS:

public Q_SLOTS:
    void handle_gate_focused(void* sender, u32 id);

private:
    unsigned int m_max_history_size;

    std::list<u32> m_gate_id_container;

    std::list<u32>::iterator m_current_item_iterator;

    void debugPrint();

    void relay_navigation(u32 id);

};

#endif    // SELECTION_HISTORY_NAVIGATOR_H