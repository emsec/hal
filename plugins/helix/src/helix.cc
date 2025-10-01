#include "helix/helix.h"

#include "gui/content_manager/content_manager.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graph_context_manager.h"
#include "gui/graph_widget/layouters/net_layout_point.h"
#include "gui/gui_globals.h"
#include "gui/user_action/action_add_items_to_object.h"
#include "gui/user_action/action_create_object.h"
#include "gui/user_action/user_action_compound.h"
#include "gui/user_action/user_action_object.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "protocol/helix.pb.h"

#include <algorithm>
#include <cstring>
#include <errno.h>
#include <event2/event.h>
#include <event2/thread.h>
#include <google/protobuf/repeated_ptr_field.h>
#include <hiredis/adapters/libevent.h>
#include <hiredis/async.h>
#include <hiredis/hiredis.h>
#include <hiredis/read.h>
#include <mutex>
#include <ostream>
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <qset.h>
#include <qstring.h>
#include <stdexcept>
#include <unordered_set>
#include <utility>

#define MAX_HOST_SIZE 1024LLU

namespace hal
{
    namespace
    {
        void connect_callback( const redisAsyncContext *ctx, int status )
        {
            u16 port = 0;
            char host[MAX_HOST_SIZE] = { 0 };

            if( status != REDIS_OK )
            {
                log_error( "helix", "could not connect to redis instance (async): {}", ctx->errstr );
                return;
            }

            // Why is saddr uninitialized

            /*
            if(ctx->saddr->sa_family == AF_UNIX)
            {
                const struct sockaddr_un *addr_un = (const struct sockaddr_un *)ctx->saddr;
                strncpy(host, addr_un->sun_path, sizeof(host));
                log_info("helix", "connected to unix socket (async): '{}'", host);
                return;
            }
            else if (ctx->saddr->sa_family == AF_INET)
            {
                const struct sockaddr_in *addr_in = (const struct sockaddr_in *)ctx->saddr;
                inet_ntop(AF_INET, &(addr_in->sin_addr), host, sizeof(host));
                port = ntohs(addr_in->sin_port);
            }
            else if (ctx->saddr->sa_family == AF_INET6)
            {
                const struct sockaddr_in6 *addr_in6 = (const struct sockaddr_in6 *)ctx->saddr;
                inet_ntop(AF_INET6, &(addr_in6->sin6_addr), host, sizeof(host));
                port = ntohs(addr_in6->sin6_port);
            }

            log_info( "helix", "connected to redis instance at {}:{} (async)", host, port );
            */
        }

        void disconnect_callback( const redisAsyncContext *ctx, int status )
        {
            if( status != REDIS_OK )
            {
                log_error( "helix", "disconnected from redis instance with error: {}", ctx->errstr );
            }
            else
            {
                log_info( "helix", "disconnected from redis instance" );
            }
        }

        [[maybe_unused]] void command_callback( redisAsyncContext *ctx, void *reply, void *privdata )
        {
            (void) ctx;
            const char *tag = (const char *) privdata;

            if( reply == nullptr )
            {
                log_warning( "helix", "[{}] <null>", tag );
                return;
            }

            redisReply *redis_reply = (redisReply *) reply;
            switch( redis_reply->type )
            {
                case REDIS_REPLY_STATUS:
                case REDIS_REPLY_STRING:
                    log_debug( "helix", "[{}] {}", tag, redis_reply->str );
                    break;
                case REDIS_REPLY_INTEGER:
                    log_debug( "helix", "[{}] {}", tag, redis_reply->integer );
                    break;
                case REDIS_REPLY_NIL:
                    log_debug( "helix", "[{}] <nil>", tag );
                    break;
                case REDIS_REPLY_ARRAY:
                    log_debug( "helix", "[{}] <array>", tag );
                    break;
                default:
                    log_debug( "helix", "[{}] reply type: {}", tag, redis_reply->type );
            }
        }
    }  // namespace

    namespace helix
    {
        namespace
        {
            QSet<u32> strings_to_set( const std::vector<std::string> &strings )
            {
                QSet<u32> gate_ids;
                for( const std::string &string : strings )
                {
                    bool ok;
                    QString qstring = QString::fromStdString( string );
                    u32 gate_id = qstring.toUInt( &ok );
                    if( ok )
                    {
                        gate_ids.insert( gate_id );
                    }
                    else
                    {
                        log_error( "helix", "could not convert {} to u32", string );
                    }
                }
                return gate_ids;
            }

            void handle_gate_action_isolate( const Netlist *netlist, const GateActionIsolate &msg )
            {
                if( netlist == nullptr )
                {
                    log_error( "helix", "no netlist provided" );
                    return;
                }

                if( msg.identifiers().empty() )
                {
                    log_warning( "helix", "GateActionIsolate message does not contain any identifiers" );
                    return;
                }

                QSet<u32> gate_ids;
                std::unordered_set<std::string> identifiers( msg.identifiers().begin(), msg.identifiers().end() );
                for( const Gate *gate : netlist->get_gates() )
                {
                    if( std::find( identifiers.begin(), identifiers.end(), gate->get_name() ) != identifiers.end() )
                    {
                        gate_ids.insert( gate->get_id() );
                    }
                }

                QMetaObject::invokeMethod(
                    gGraphContextManager,
                    [gate_ids]() {
                        QString name = gGraphContextManager->nextViewName( "Helix View" );
                        UserActionCompound *act = new UserActionCompound;
                        act->setUseCreatedObject();
                        act->addAction( new ActionCreateObject( UserActionObjectType::ContextView, name ) );
                        act->addAction( new ActionAddItemsToObject( QSet<u32>(), gate_ids ) );
                        act->exec();
                        GraphContext *context = gGraphContextManager->getContextById( act->object().id() );
                        if( context )
                            context->setDirty( false );
                    },
                    Qt::QueuedConnection );
            }

            void handle_gate_action_zoom( const Netlist *netlist, const GateActionZoom &msg )
            {
                if( netlist == nullptr )
                {
                    log_error( "helix", "no netlist provided" );
                    return;
                }

                if( msg.identifiers().empty() )
                {
                    log_warning( "helix", "GateActionZoom message does not contain any identifiers" );
                    return;
                }

                std::vector<u32> gate_ids;
                std::unordered_set<std::string> identifiers( msg.identifiers().begin(), msg.identifiers().end() );
                for( const Gate *gate : netlist->get_gates() )
                {
                    if( std::find( identifiers.begin(), identifiers.end(), gate->get_name() ) != identifiers.end() )
                    {
                        gate_ids.push_back( gate->get_id() );
                    }
                }

                if( gate_ids.size() != 1 )
                {
                    log_info( "helix",
                              "GateActionZoom message contains more than one identifier; fallback to isolation" );
                    GateActionIsolate new_msg;
                    *new_msg.mutable_identifiers() = msg.identifiers();
                    handle_gate_action_isolate( netlist, new_msg );
                    return;
                }

                u32 gate_id = gate_ids.at( 0 );
                QMetaObject::invokeMethod(
                    gContentManager,
                    [gate_id]() { gContentManager->getGraphTabWidget()->handleGateFocus( gate_id ); },
                    Qt::QueuedConnection );
            }

            void handle_gate_action_select( const Netlist *netlist, const GateActionSelect &msg )
            {
                if( netlist == nullptr )
                {
                    log_error( "helix", "no netlist provided" );
                    return;
                }

                if( msg.identifiers().empty() )
                {
                    log_warning( "helix", "GateActionZoom message does not contain any identifiers" );
                    return;
                }

                throw std::logic_error( "Not implemented" );
            }

            void handle_create_module( const Netlist *netlist, const CreateModule &msg )
            {
                if( netlist == nullptr )
                {
                    log_error( "helix", "no netlist provided" );
                    return;
                }

                throw std::logic_error( "Not implemented" );
            }

            void handle_delete_module( const Netlist *netlist, const DeleteModule &msg )
            {
                if( netlist == nullptr )
                {
                    log_error( "helix", "no netlist provided" );
                    return;
                }

                throw std::logic_error( "Not implemented" );
            }

            void subscriber_callback( redisAsyncContext *ctx, void *reply, void *privdata )
            {
                (void) ctx;
                Netlist *netlist = (Netlist *) privdata;
                redisReply *msg = (redisReply *) reply;

                if( reply == nullptr )
                {
                    log_warning( "helix", "empty reply from redis subscription" );
                    return;
                }

                if( netlist == nullptr )
                {
                    log_error( "helix", "no netlist provided" );
                    return;
                }

                if( msg->type != REDIS_REPLY_ARRAY )
                {
                    return;
                }

                if( strcmp( msg->element[0]->str, "message" ) != 0 )
                {
                    return;
                }

                Message message;
                if( !message.ParseFromString( msg->element[2]->str ) )
                {
                    log_error( "helix", "failed to parse message" );
                    return;
                }

                switch( message.payload_case() )
                {
                    case Message::kGateActionZoom:
                        handle_gate_action_zoom( netlist, message.gate_action_zoom() );
                        break;
                    case Message::kGateActionSelect:
                        handle_gate_action_select( netlist, message.gate_action_select() );
                        break;
                    case Message::kGateActionIsolate:
                        handle_gate_action_isolate( netlist, message.gate_action_isolate() );
                        break;
                    case Message::kCreateModule:
                        handle_create_module( netlist, message.create_module() );
                        break;
                    case Message::kDeleteModule:
                        handle_delete_module( netlist, message.delete_module() );
                        break;
                }
            }

            void subscriber( const Netlist *netlist,
                             redisAsyncContext *ctx,
                             const std::vector<std::string> &channels,
                             struct event_base *base )
            {
                if( netlist == nullptr )
                {
                    log_error( "helix", "no netlist provided" );
                    return;
                }

                if( base == nullptr )
                {
                    log_error( "helix", "no event base provided" );
                    return;
                }

                std::ostringstream oss;
                oss << "";
                for( auto &ch : channels )
                    oss << " " << ch;
                std::string cmd = "SUBSCRIBE " + oss.str();

                if( redisAsyncCommand( ctx, subscriber_callback, (void *) netlist, cmd.c_str() ) != REDIS_OK )
                {
                    log_error( "helix", "TODO" );
                    return;
                }

                // TODO: signal main thread to cleanup resources if errors occurr
                if( event_base_dispatch( base ) != 0 )
                {
                    char *errstr = strerror( errno );
                    log_error( "helix", "event_base_dispatch: {}", errstr );
                    return;
                }
            }
        }  // namespace

        Helix *Helix::inst = nullptr;
        std::string Helix::channel = "hal";

        Helix *Helix::instance()
        {
            if( inst == nullptr )
            {
                inst = new Helix;
            }
            return inst;
        }

        Helix::Helix()
        {
            m_is_running = false;
            m_netlist = nullptr;
            m_base = nullptr;
            m_sctx = nullptr;
            m_pctx = nullptr;
        }

        void Helix::start( const Netlist *netlist,
                           const std::string &host,
                           const u16 port,
                           const std::vector<std::string> &channels )
        {
            std::lock_guard<std::mutex> lock( m_is_running_mtx );
            if( m_is_running )
            {
                log_warning( "helix", "helix is already running" );
                return;
            }

            if( netlist == nullptr )
            {
                log_error( "helix", "no netlist provided" );
                return;
            }
            else if( *netlist != *gNetlist )
            {
                log_warning( "helix", "provided netlist differs from netlist in gui" );
            }

            if( evthread_use_pthreads() != 0 )
            {
                char *errstr = strerror( errno );
                log_error( "helix", "evthread_use_pthreads: {}", errstr );
                return;
            }

            m_base = event_base_new();
            if( m_base == nullptr )
            {
                char *errstr = strerror( errno );
                log_error( "helix", "event_base_new: {}", errstr );
                return;
            }

            redisAsyncContext *m_sctx = redisAsyncConnect( host.c_str(), port );
            if( m_sctx->err )
            {
                log_error(
                    "helix", "redisAsyncConnect: {}", ( m_sctx ? m_sctx->errstr : "context allocation failed" ) );
                event_base_free( m_base );
                return;
            }

            redisAsyncSetConnectCallback( m_sctx, connect_callback );
            redisAsyncSetDisconnectCallback( m_sctx, disconnect_callback );

            m_pctx = redisConnect( host.c_str(), port );
            if( m_pctx == nullptr || m_pctx->err )
            {
                log_error( "helix", "redisConnect: {}", ( m_pctx ? m_pctx->errstr : "context allocation failed" ) );
                redisAsyncDisconnect( m_sctx );
                event_base_free( m_base );
                return;
            }

            log_info( "helix", "connected to redis instance at {}:{}", host, port );

            if( redisLibeventAttach( m_sctx, m_base ) != REDIS_OK )
            {
                log_error( "helix", "redisLibeventAttach: {}", m_sctx->errstr );
                redisAsyncDisconnect( m_sctx );
                event_base_free( m_base );
                redisFree( m_pctx );
                return;
            }

            m_subscriber = std::thread( subscriber, netlist, m_sctx, channels, m_base );
            m_is_running = true;

            log_info( "helix", "started subscriber thread" );
        }

        void Helix::stop()
        {
            std::lock_guard<std::mutex> lock( m_is_running_mtx );
            if( !m_is_running )
            {
                log_info( "helix", "helix is not running" );
                return;
            }

            if( event_base_loopbreak( m_base ) != 0 )
            {
                char *errstr = strerror( errno );
                log_error( "helix", "event_base_loopbreak: {}", errstr );
                return;
            }

            m_subscriber.join();
            m_is_running = false;

            log_info( "helix", "subscriber thread stopped" );

            redisFree( m_pctx );

            log_info( "helix", "helix shut down" );
        }

        bool Helix::publish( const std::string &channel, const std::string &message )
        {
            bool ret = true;
            const auto reply =
                static_cast<redisReply *>( redisCommand( m_pctx, "PUBLISH %s %s", channel.c_str(), message.c_str() ) );
            if( reply == nullptr || m_pctx->err )
            {
                log_error( "helix", "redisCommand: {}", m_pctx->errstr );
                ret = false;
            }
            freeReplyObject( reply );
            return ret;
        }

        const Netlist *Helix::get_netlist() const
        {
            return m_netlist;
        }

        const bool Helix::is_running() const
        {
            std::lock_guard<std::mutex> lock( m_is_running_mtx );
            return m_is_running;
        }
    }  // namespace helix
}  // namespace hal
