#ifndef SELECTION_HISTORY_NAVIGATOR_H
#define SELECTION_HISTORY_NAVIGATOR_H

#include "def.h"
#include "selection_history_navigator/selection.h"

#include <QObject>
#include <list>

class selection_history_navigator : public QObject
{
    Q_OBJECT

public:
    selection_history_navigator(unsigned int max_history_size, QObject* parent = 0);
    ~selection_history_navigator();
    
    void navigate_to_prev_item();
    void navigate_to_next_item();

    void set_max_history_size(unsigned int max_size);
    u32 get_max_history_size() const;

Q_SIGNALS:

public Q_SLOTS:
    void handle_gate_focused(void* sender, u32 id);
    void handle_net_focused(void* sender, u32 id);

private:
    unsigned int m_max_history_size;

    std::list<selection> m_selection_container;

    std::list<selection>::iterator m_current_item_iterator;

    void relay_selection(selection selection);

    void store_selection(u32 id, selection_relay::item_type type);
};

#endif    // SELECTION_HISTORY_NAVIGATOR_H
