#pragma  once
#include  "hal_core/defines.h"
#include  "hal_core/plugin_system/plugin_interface_base.h"
#include  "hal_core/utilities/result.h"
#include  "hal_core/netlist/netlist.h"

namespace  hal
{
	class  PLUGIN_API  NetlistModifierPlugin : public  BasePluginInterface
	{
	public:
		std::string  get_name() const  override;
		std::string  get_version() const  override;
		void  initialize() override;

        NetlistModifierPlugin(){};

        bool  create_modified_netlist();

        Netlist* modified_netlist_pointer = nullptr;
    private:
        
        bool replace_gates();
	};
} // namespace hal