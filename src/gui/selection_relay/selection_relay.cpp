#include "selection_relay/selection_relay.h"
#include "core/log.h"

selection_relay::selection_relay(QObject* parent) : QObject(parent)
{
}

void selection_relay::register_sender(void* sender, QString name)
{
    m_sender_register.append(QPair<void*, QString>(sender, name));
}

void selection_relay::remove_sender(void* sender)
{
    for (QPair<void*, QString> pair : m_sender_register)
    {
        if (pair.first == sender)
            m_sender_register.removeOne(pair);
    }
}

void selection_relay::relay_gate_selection(void* sender, QList<u32>& gate_ids, Mode mode)
{
    Q_EMIT gate_selection_update(sender, gate_ids, mode);
}

void selection_relay::relay_net_selection(void* sender, QList<u32>& net_ids, Mode mode)
{
    Q_EMIT net_selection_update(sender, net_ids, mode);
}

void selection_relay::relay_module_selection(void* sender, QList<u32>& module_ids, Mode mode)
{
    Q_EMIT module_selection_update(sender, module_ids, mode);
}

void selection_relay::relay_combined_selection(void* sender, QList<u32>& gate_ids, QList<u32>& net_ids, QList<u32>& module_ids, Mode mode)
{
    QString name = "unknown";
    for (QPair<void*, QString> pair : m_sender_register)
    {
        if (pair.first == sender)
            name = pair.second;
    }
    QString string = name + " selection: ";
    switch (mode)
    {
        case Mode::override:
            string += "Override ";
            break;
        case Mode::add:
            string += "Add ";
            break;
        case Mode::remove:
            string += "Remove ";
            break;
    }

    int num_gates = gate_ids.length();

    if (num_gates)
    {
        string += QString::number(num_gates) + " gates (";

        QList<u32>::const_iterator iterator;
        iterator = gate_ids.begin();
        int i    = num_gates - 1;
        while (i)
        {
            string += QString::number(*iterator) + ", ";
            iterator++;
            i--;
        }
        string += QString::number(*iterator) + "), ";
    }
    else
        string += "0 gates, ";

    int num_nets = net_ids.length();

    if (num_nets)
    {
        string += QString::number(num_nets) + " nets (";

        QList<u32>::const_iterator iterator;
        iterator = net_ids.begin();
        int i    = num_nets - 1;
        while (i)
        {
            string += QString::number(*iterator) + ", ";
            iterator++;
            i--;
        }
        string += QString::number(*iterator) + "), ";
    }
    else
        string += "0 nets, ";

    int num_modules = module_ids.length();

    if (num_modules)
    {
        string += QString::number(num_modules) + " modules (";

        QList<u32>::const_iterator iterator;
        iterator = module_ids.begin();
        int i    = num_modules - 1;
        while (i)
        {
            string += QString::number(*iterator) + ", ";
            iterator++;
            i--;
        }
        string += QString::number(*iterator) + ")";
    }
    else
        string += "0 modules";

    log_info("user", string.toStdString());

    Q_EMIT combined_selection_update(sender, gate_ids, net_ids, module_ids, mode);
}

void selection_relay::relay_current_gate(void* sender, u32 gate_id)
{
    Q_EMIT current_gate_update(sender, gate_id);
}

void selection_relay::relay_current_net(void* sender, u32 net_id)
{
    Q_EMIT current_net_update(sender, net_id);
}

void selection_relay::relay_current_module(void* sender, u32 module_id)
{
    Q_EMIT current_module_update(sender, module_id);
}

void selection_relay::relay_current_cleared(void* sender)
{
    Q_EMIT current_cleared_update(sender);
}

void selection_relay::relay_current_deleted(void* sender)
{
    Q_EMIT current_deleted_update(sender);
}

void selection_relay::relay_jump_gate(void* sender, u32 gate_id)
{
    Q_EMIT jump_gate_update(sender, gate_id);
}

void selection_relay::relay_jump_net(void* sender, u32 net_id)
{
    Q_EMIT jump_net_update(sender, net_id);
}

void selection_relay::relay_jump_module(void* sender, u32 module_id)
{
    Q_EMIT jump_module_update(sender, module_id);
}

void selection_relay::relay_jump_selection(void* sender)
{
    Q_EMIT jump_selection_update(sender);
}

void selection_relay::relay_gate_highlight(void* sender, QList<u32>& ids, Mode mode, u32 channel)
{
    Q_EMIT gate_highlight_update(sender, ids, mode, channel);
}

void selection_relay::relay_net_highlight(void* sender, QList<u32>& ids, Mode mode, u32 channel)
{
    Q_EMIT net_highlight_update(sender, ids, mode, channel);
}

void selection_relay::relay_module_highlight(void* sender, QList<u32>& ids, Mode mode, u32 channel)
{
    Q_EMIT module_highlight_update(sender, ids, mode, channel);
}

void selection_relay::relay_combined_highlight(void* sender, QList<u32>& gate_ids, QList<u32>& net_ids, QList<u32>& module_ids, Mode mode, u32 channel)
{
    Q_EMIT combined_highlight_update(sender, gate_ids, net_ids, module_ids, mode, channel);
}
