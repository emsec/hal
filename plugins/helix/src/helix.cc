#include "helix/helix.h"
#include "helix/redis_communication.h"

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

#include <assert.h>
#include <errno.h>
#include <hiredis/async.h>
#include <hiredis/hiredis.h>
#include <hiredis/read.h>
#include <mutex>
#include <QJsonDocument>
#include <QJsonArray>
#include <QString>

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

        Helix::Helix(QObject *parent)
            : QObject(parent),
              m_is_running(false), m_redis_communication(nullptr)
        {;}

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

            m_redis_communication = new RedisCommunication(this);  // child QObject should get deleted on parent destructor

            m_redis_communication->rememberNetlist(netlist);
            connect(m_redis_communication, &RedisCommunication::messageReceived, this, &Helix::handle_message_received);
            connect(m_redis_communication, &RedisCommunication::errorMessage, this, &Helix::handle_redis_error);

            m_redis_communication->subscribeToChannels(host,port,channels);


            m_is_running = true;

            log_info( "helix", "start and subscribe to channel done" );
        }

        void Helix::stop()
        {
            std::lock_guard<std::mutex> lock( m_is_running_mtx );
            if( !m_is_running )
            {
                log_info( "helix", "helix is not running" );
                return;
            }

            m_is_running = false;

            log_info( "helix", "subscriber thread stopped" );

            log_info( "helix", "helix shut down" );
        }

        const bool Helix::is_running() const
        {
            std::lock_guard<std::mutex> lock( m_is_running_mtx );
            return m_is_running;
        }

        void Helix::handle_message_received(const QString& msg, const QString& channel)
        {
            QJsonParseError parse_error;

            QJsonDocument payload_json = QJsonDocument::fromJson( msg.toUtf8(), &parse_error );

            if( parse_error.error != QJsonParseError::NoError )
            {
                log_error( "helix", "QJsonDocument::fromJson: {}", parse_error.errorString().toStdString() );
                return;
            }

            assert( payload_json.isObject() );

            const QJsonObject payload = payload_json.object();
            const std::string command = payload["command"].toString().toStdString();

            log_info( "helix", "received command '{}' on channel '{}'", command, channel.toStdString() );

            const std::vector<u32> instances = get_instance_ids( gNetlist, payload["instances"].toArray() );

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

        void Helix::handle_redis_error(const QString& errMsg)
        {
            log_warning( "helix", "redis error '{}'", errMsg.toStdString());
        }

        RedisCommunication* Helix::get_redis_communication() const
        {
            return m_redis_communication;
        }

    }  // namespace helix
}  // namespace hal
