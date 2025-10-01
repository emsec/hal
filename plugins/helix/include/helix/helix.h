#pragma once

#include "hal_core/defines.h"

#include <event2/event.h>
#include <hiredis/async.h>
#include <hiredis/hiredis.h>
#include <string>
#include <thread>
#include <vector>

namespace hal
{
    class Netlist;
}

namespace hal
{
    namespace helix
    {
        class Helix
        {
          public:
            static std::string channel;

            Helix();

            ~Helix() = default;

            static Helix *instance();

            void start( const Netlist *netlist,
                        const std::string &host,
                        const u16 port,
                        const std::vector<std::string> &channels );

            void stop();

            bool publish( const std::string &channel, const std::string &message );

            const Netlist *get_netlist() const;

            const bool is_running() const;

          private:
            static Helix *inst;

            Netlist *m_netlist;

            bool m_is_running;

            std::thread m_subscriber;

            redisAsyncContext *m_sctx;

            redisContext *m_pctx;

            struct event_base *m_base;
        };
    }  // namespace helix
}  // namespace hal
