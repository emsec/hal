#ifndef MODULE_RELAY_H
#define MODULE_RELAY_H

#include "def.h"

#include "netlist/event_system/module_event_handler.h"

#include "module_model/module_item.h"
#include "module_model/module_model.h"

#include <QColor>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QVector>

class module_relay : public QObject
{
    Q_OBJECT

    struct module_pin
    {
        std::string name;
        u32 net;
    };

    // PROBABLY OBSOLETE, INTEGRATE INTO ITEMS
    struct module_extension
    {
        module_extension(const u32 module_id) : id(module_id) {}
        ~module_extension();

        u32 id;
        QSet<module_pin> input_pins; // UNSURE
        QSet<module_pin> output_pins; // UNSURE
        bool isolated = true; // UNSURE
        QColor color = QColor(255, 255, 255);
        bool compressed = false;
        bool hidden = false;
        QVector<u32> stray_nets; // UNSURE
    };

public:
    explicit module_relay(QObject* parent = nullptr);

    void add_selection_to_module(const u32 id);

    void set_module_color(const u32 id, const QColor& color);
    void set_module_compressed(const u32 id, const bool compressed);
    void set_module_hidden(const u32 id, const bool hidden);

public Q_SLOTS:
    void handle_module_event(module_event_handler::event ev, std::shared_ptr<module> object, u32 associated_data);

private:
    QMap<u32, module_item*> m_module_items;
    module_model* m_model;
};

#endif // MODULE_RELAY_H
