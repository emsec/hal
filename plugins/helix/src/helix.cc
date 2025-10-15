#include "helix/helix.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graph_context_manager.h"
#include "gui/graph_widget/layouters/net_layout_point.h"
#include "gui/gui_api/gui_api.h"
#include "gui/gui_globals.h"
#include "gui/user_action/action_add_items_to_object.h"
#include "gui/user_action/action_create_object.h"
#include "gui/user_action/user_action_compound.h"
#include "gui/user_action/user_action_object.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "qjsonarray.h"

#include <algorithm>
#include <assert.h>
#include <cstring>
#include <errno.h>
#include <event2/event.h>
#include <event2/thread.h>
#include <future>
#include <hiredis/adapters/libevent.h>
#include <hiredis/async.h>
#include <hiredis/hiredis.h>
#include <hiredis/read.h>
#include <mutex>
#include <ostream>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonvalue.h>
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <qset.h>
#include <qstring.h>
#include <unordered_set>
#include <utility>

namespace hal
{
    namespace
    {
        void connect_callback( const redisAsyncContext *ctx, int status )
        {
            if( status != REDIS_OK )
            {
                log_error( "helix", "could not connect to redis instance (async): {}", ctx->errstr );
                return;
            }
            log_info( "helix", "connected to redis instance (async)" );
        }

        void disconnect_callback( const redisAsyncContext *ctx, int status )
        {
            if( status != REDIS_OK )
            {
                log_error( "helix", "disconnected from redis instance (async) with error: {}", ctx->errstr );
            }
            else
            {
                log_info( "helix", "disconnected from redis instance (async)" );
            }
        }

        const std::string build_subscribe_command( const std::vector<std::string> &channels )
        {
            std::ostringstream oss;
            oss << "";
            for( const std::string &ch : channels )
                oss << " " << ch;
            return "SUBSCRIBE " + oss.str();
        }

        const std::vector<u32> get_instance_ids( const Netlist *netlist, const QJsonArray &instances )
        {
            std::vector<u32> instance_ids;
            std::unordered_set<std::string> instance_names;

            for( const QJsonValue &value : instances )
            {
                instance_names.insert( value.toString().toStdString() );
            }

            for( const Gate *gate : netlist->get_gates() )
            {
                if( std::find( instance_names.begin(), instance_names.end(), gate->get_name() )
                    != instance_names.end() )
                {
                    instance_ids.push_back( gate->get_id() );
                }
            }

            return instance_ids;
        }

        void handle_gate_action_isolate( const std::vector<u32> &instance_ids )
        {
            const QSet<u32> instance_ids_qset( instance_ids.begin(), instance_ids.end() );
            QMetaObject::invokeMethod(
                gGraphContextManager,
                [instance_ids_qset]() {
                    QString name = gGraphContextManager->nextViewName( "Helix View" );
                    UserActionCompound *act = new UserActionCompound;
                    act->setUseCreatedObject();
                    act->addAction( new ActionCreateObject( UserActionObjectType::ContextView, name ) );
                    act->addAction( new ActionAddItemsToObject( QSet<u32>(), instance_ids_qset ) );
                    act->exec();
                    GraphContext *context = gGraphContextManager->getContextById( act->object().id() );
                    if( context )
                        context->setDirty( false );
                },
                Qt::QueuedConnection );
            gGuiApi->selectGate( instance_ids, true, false );
        }

        void handle_gate_action_zoom( const std::vector<u32> &instance_ids )
        {
            gGuiApi->selectGate( instance_ids, true, true );
        }
    }  // namespace

    namespace helix
    {
        namespace
        {
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

                std::string msg_type{ msg->element[0]->str };
                std::string channel{ msg->element[1]->str };

                if( msg_type != "message" )
                {
                    return;
                }

                std::string payload_json_str{ msg->element[2]->str };

                QJsonParseError parse_error;
                QString payload_json_qstr = QString::fromStdString( payload_json_str );
                QJsonDocument payload_json = QJsonDocument::fromJson( payload_json_qstr.toUtf8(), &parse_error );

                if( parse_error.error != QJsonParseError::NoError )
                {
                    log_error( "helix", "QJsonDocument::fromJson: {}", parse_error.errorString().toStdString() );
                    return;
                }

                assert( payload_json.isObject() );

                const QJsonObject payload = payload_json.object();
                const std::string command = payload["command"].toString().toStdString();

                log_info( "helix", "received command '{}' on channel '{}'", command, channel );

                const std::vector<u32> instances = get_instance_ids( netlist, payload["instances"].toArray() );

                if( instances.size() == 0 )
                {
                    log_warning( "helix", "no instances found" );
                    return;
                }

                if( command == "GateActionZoom" )
                {
                    handle_gate_action_zoom( instances );
                }
                else if( command == "GateActionIsolate" )
                {
                    handle_gate_action_isolate( instances );
                }
                else
                {
                    log_warning( "helix", "received invalid command '{}'", command );
                }
            }

            void subscriber( const Netlist *netlist,
                             redisAsyncContext *ctx,
                             const std::vector<std::string> &channels,
                             struct event_base *base,
                             std::promise<bool> subscribe_promise )
            {
                if( netlist == nullptr )
                {
                    log_error( "helix", "no netlist provided" );
                    subscribe_promise.set_value( false );
                    return;
                }

                if( ctx == nullptr )
                {
                    log_error( "helix", "no redis context provided" );
                    subscribe_promise.set_value( false );
                    return;
                }

                if( base == nullptr )
                {
                    log_error( "helix", "no event base provided" );
                    subscribe_promise.set_value( false );
                    return;
                }

                std::string subscribe_command = build_subscribe_command( channels );

                if( redisAsyncCommand( ctx, subscriber_callback, (void *) netlist, subscribe_command.c_str() )
                    != REDIS_OK )
                {
                    log_error( "helix", "async subscribe to channels {} failed", subscribe_command.substr( 10 ) );
                    subscribe_promise.set_value( false );
                    return;
                }

                // I just assume that the dispath won't fail
                subscribe_promise.set_value( true );

                if( event_base_dispatch( base ) != 0 )
                {
                    char *errstr = strerror( errno );
                    log_error( "helix", "event_base_dispatch: {}", errstr );
                    return;
                }
            }
        }  // namespace

        Helix *Helix::inst = nullptr;
        const std::string Helix::channel = "hal";

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

        Helix::~Helix()
        {
            this->stop();
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
                redisAsyncDisconnect( m_sctx );
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
                redisFree( m_pctx );
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

            std::promise<bool> subscribe_promise;
            std::future<bool> subscribe_future = subscribe_promise.get_future();

            m_subscriber = std::thread( subscriber, netlist, m_sctx, channels, m_base, std::move( subscribe_promise ) );

            const bool success = subscribe_future.get();
            if( !success )
            {
                redisAsyncDisconnect( m_sctx );
                event_base_free( m_base );
                redisFree( m_pctx );
                return;
            }

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
