#include "helix/gui_extension_helix.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/plugin_system/plugin_parameter.h"
#include "hal_core/utilities/log.h"
#include "helix/helix.h"
#include "helix/plugin_helix.h"

#include <istream>
#include <string>
#include <vector>

namespace hal
{
    namespace
    {
        std::vector<std::string> get_channels_from_string( const std::string &value )
        {
            std::string str;
            std::vector<std::string> retval;
            std::istringstream sstream( value );
            while( std::getline( sstream, str, ' ' ) )
            {
                retval.push_back( str );
            }

            return retval;
        }
    }  // namespace

    GuiExtensionHelix::GuiExtensionHelix()
    {
    }

    std::vector<ContextMenuContribution> GuiExtensionHelix::get_context_contribution( const Netlist *nl,
                                                                                      const std::vector<u32> &mods,
                                                                                      const std::vector<u32> &gates,
                                                                                      const std::vector<u32> &nets )
    {
        std::vector<ContextMenuContribution> additions;

        additions.push_back( { this, "gate_action_zoom", "Zoom gates" } );
        additions.push_back( { this, "gate_action_select", "Select gates" } );
        additions.push_back( { this, "gate_action_isolate", "Isolate gates" } );

        return additions;
    }

    std::vector<PluginParameter> GuiExtensionHelix::get_parameter() const
    {
        std::vector<PluginParameter> retval;
        retval.push_back( PluginParameter( PluginParameter::String, "host", "Host", "localhost" ) );
        retval.push_back( PluginParameter( PluginParameter::Integer, "port", "Port", "6379" ) );
        retval.push_back(
            PluginParameter( PluginParameter::String, "channels", "Channels", "application1 application2" ) );
        retval.push_back( PluginParameter( PluginParameter::PushButton, "start", "Start" ) );
        retval.push_back( PluginParameter( PluginParameter::PushButton, "stop", "Stop" ) );
        return retval;
    }

    void GuiExtensionHelix::set_parameter( const std::vector<PluginParameter> &argv )
    {
        for( const PluginParameter &arg : argv )
        {
            if( arg.get_tagname() == "stop" && arg.get_value() == "clicked" )
            {
                this->m_parent->get_helix()->stop();
                return;
            }
            if( arg.get_tagname() == "host" )
            {
                m_host = arg.get_value();
            }
            else if( arg.get_tagname() == "port" )
            {
                m_port = std::stoi( arg.get_value() );
            }
            else if( arg.get_tagname() == "channels" )
            {
                m_channels = get_channels_from_string( arg.get_value() );
            }
            else if( arg.get_tagname() == "start" )
            {
                m_button_clicked = ( arg.get_value() == "clicked" );
            }
        }

        if( m_button_clicked )
        {
            this->m_parent->get_helix()->start( gNetlist, m_host, m_port, m_channels );
            m_button_clicked = false;
        }
    }

    void GuiExtensionHelix::execute_function( std::string tag,
                                              Netlist *nl,
                                              const std::vector<u32> &mods,
                                              const std::vector<u32> &gats,
                                              const std::vector<u32> &nets )
    {
        if( tag == "start" || tag == "stop" )
        {
            return;
        }

        if( nl == nullptr )
        {
            log_error( "helix", "no netlist provided" );
            return;
        }

        if( !this->m_parent->get_helix()->is_running() )
        {
            log_error( "helix", "helix is not running" );
            return;
        }

        if( mods.empty() && gats.empty() )
        {
            log_warning( "helix", "{}: no gates provided", tag );
            return;
        }

        std::set<std::string> identifiers;
        for( const u32 module_id : mods )
        {
            Module *module = nl->get_module_by_id( module_id );

            if( module == nullptr )
            {
                log_warning( "helix", "no module with id {} found in netlist", module_id );
                continue;
            }

            const std::vector<Gate *> &gates = module->get_gates( []( Gate *g ) { return true; }, true );

            for( const Gate *gate : gates )
            {
                identifiers.insert( gate->get_name() );
            }
        }

        for( const auto gate_id : gats )
        {
            Gate *gate = nl->get_gate_by_id( gate_id );

            if( gate == nullptr )
            {
                log_warning( "helix", "no gate with id {} found in netlist", gate_id );
                continue;
            }

            identifiers.insert( gate->get_name() );
        }

        if( identifiers.empty() )
        {
            log_warning( "helix", "no valid gates provided" );
            return;
        }

        if( tag == "gate_action_zoom" )
        {
        }
        else if( tag == "gate_action_select" )
        {
        }
        else if( tag == "gate_action_isolate" )
        {
        }

        std::string payload;
        this->m_parent->get_helix()->publish( helix::Helix::channel, payload );
    }
}  // namespace hal
