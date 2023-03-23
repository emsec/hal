#include "hal_core/plugin_system/plugin_manager.h"

#include "hal_core/plugin_system/plugin_interface_ui.h"
#include "hal_core/plugin_system/fac_extension_interface.h"
#include "hal_core/plugin_system/cli_extension_interface.h"
#include "hal_core/plugin_system/runtime_library.h"
#include "hal_core/netlist/gate_library/gate_library_parser/gate_library_parser_manager.h"
#include "hal_core/netlist/gate_library/gate_library_writer/gate_library_writer_manager.h"
#include "hal_core/netlist/netlist_parser/netlist_parser_manager.h"
#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

#include <vector>

#ifdef _WIN32
#include <tchar.h>
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

namespace hal
{
    namespace plugin_manager
    {
        namespace
        {
            // stores library and factory identified by plugin name)
            std::unordered_map<std::string, std::tuple<std::unique_ptr<BasePluginInterface>, std::unique_ptr<RuntimeLibrary>>> m_loaded_plugins;

            // stores special features offered by plugin
            std::unordered_map<std::string, std::vector<PluginFeature>> m_plugin_features;

            // stores the CLI parser option for plugins [0=base_plugin] [1=UI_plugin]
            std::unordered_map<std::string, std::string> m_cli_option_to_plugin_name[2];

            // stores the GUI callback
            CallbackHook<void(bool, std::string const&, std::string const&)> m_hook;

            // stores the generic option parser
            ProgramOptions m_existing_options("existing options");

            // stores the plugin option description
            ProgramOptions m_plugin_options("plugin options");

            // stores the plugin folder
            std::vector<std::filesystem::path> m_plugin_folders = utils::get_plugin_directories();

            // stores name of plugin while loading
            std::string m_current_loading;

            std::filesystem::path get_plugin_path(std::string plugin_name)
            {
                std::filesystem::path retval;
                if (plugin_name.empty()) return retval;
                std::string file_name = plugin_name + "." + LIBRARY_FILE_EXTENSION;
                retval = utils::get_file(file_name, m_plugin_folders);
                if (!retval.empty() || !strlen(ALTERNATE_LIBRARY_FILE_EXTENSION)) return retval;
                file_name = plugin_name + "." + ALTERNATE_LIBRARY_FILE_EXTENSION;
                return utils::get_file(file_name, m_plugin_folders);
            }

            bool solve_dependencies(std::string plugin_name, std::set<std::string> dep_file_name)
            {
                if (plugin_name.empty())
                {
                    log_error("core", "parameter 'plugin_name' is empty");
                    return false;
                }
                if (dep_file_name.empty())
                {
                    log_debug("core", "no dependency for plugin '{}'", plugin_name);
                    return true;
                }
                for (const auto& file_name : dep_file_name)
                {
                    auto dep_plugin_name        = std::filesystem::path(file_name).stem().string();

                    // dependency already loaded
                    if (m_loaded_plugins.find(dep_plugin_name) != m_loaded_plugins.end())
                    {
                        continue;
                    }

                    // search in plugin directories
                    std::filesystem::path file_path = get_plugin_path(dep_plugin_name);
                    if (file_path.empty() || !load(dep_plugin_name, file_path))
                    {
                        log_error("core", "cannot solve dependency '{}' for plugin '{}'", dep_plugin_name, plugin_name);
                        return false;
                    }

                    log_debug("core", "solved dependency '{}' for plugin '{}'", dep_plugin_name, plugin_name);
                }
                log_debug("core", "solved {} dependencies for plugin '{}'", dep_file_name.size(), plugin_name);
                return true;
            }

        }    // namespace

        bool has_valid_file_extension(std::filesystem::path file_name)
        {
#if defined(__APPLE__) && defined(__MACH__)
            if (utils::ends_with(file_name.string(), std::string(".so")))
                return true;
            if (file_name.string().find(".so") != std::string::npos)
                return true;
            if (utils::ends_with(file_name.string(), std::string(".icloud")))
                return false;
#endif
            // file has regular shared library extension
            if (utils::ends_with(file_name.string(), std::string(".") + std::string(LIBRARY_FILE_EXTENSION))) return true;

            // there is no alternate extension
            if (!strlen(ALTERNATE_LIBRARY_FILE_EXTENSION)) return false;

            // test whether file has alternate extension
            return (utils::ends_with(file_name.string(), std::string(".") + std::string(ALTERNATE_LIBRARY_FILE_EXTENSION)));
        }

        std::set<std::string> get_plugin_names()
        {
            std::set<std::string> names;
            for (const auto& it : m_loaded_plugins)
            {
                names.insert(it.first);
            }
            return names;
        }

        std::vector<PluginFeature> get_plugin_features(std::string name)
        {
            auto it = m_plugin_features.find(name);
            if (it == m_plugin_features.end()) return std::vector<PluginFeature>();
            return it->second;
        }

        std::unordered_map<std::string, std::string> get_cli_plugin_flags()
        {
            return m_cli_option_to_plugin_name[0];
        }

        std::unordered_map<std::string, std::string> get_ui_plugin_flags()
        {
            return m_cli_option_to_plugin_name[1];
        }

        ProgramOptions get_cli_plugin_options()
        {
            return m_plugin_options;
        }

        bool load_all_plugins(const std::vector<std::filesystem::path>& directory_names)
        {
            auto directories = (!directory_names.empty()) ? directory_names : m_plugin_folders;
            for (const auto& directory : directories)
            {
                if (!std::filesystem::exists(directory))
                {
                    continue;
                }
                u32 num_of_loaded_plugins = 0;
                for (const auto& file : utils::DirectoryRange(directory))
                {
                    if (!has_valid_file_extension(file) || std::filesystem::is_directory(file))
                        continue;

                    auto plugin_name = file.path().stem().string();
                    if (load(plugin_name, file.path()))
                    {
                        num_of_loaded_plugins++;
                    }
                }
                log_debug("core", "loaded {} plugins from '{}'", num_of_loaded_plugins, directory.string());
            }
            return true;
        }

        bool load(const std::string& plugin_name, const std::filesystem::path& file_path_or_empty)
        {
            if (plugin_name.empty())
            {
                log_error("core", "parameter 'plugin_name' is empty");
                return false;
            }

            std::filesystem::path file_path = file_path_or_empty;
            if (file_path.empty())
            {
                // file name not provided, search plugin folders
                file_path = get_plugin_path(plugin_name);

                if (file_path.empty())
                {
                    log_error("core", "path for plugin '{}' not found", plugin_name);
                    return false;
                }
            }
            else
                log_info("core", "loading plugin '{}'...", file_path.string());

            if (m_loaded_plugins.find(plugin_name) != m_loaded_plugins.end())
            {
                log_debug("core", "plugin '{}' is already loaded", plugin_name);
                return true;
            }

            /* load library */
            auto lib = std::make_unique<RuntimeLibrary>();
            if (!lib->load_library(file_path.string()))
            {
                return false;
            }

            /* get factory of library */
            auto factory = (instantiate_plugin_function)lib->get_function("create_plugin_instance");
            if (factory == nullptr)
            {
                log_error("core", "file does not seem to be a HAL plugin since it does not contain a factory function 'create_plugin_instance'.");
                return false;
            }
            auto instance = factory();
            if (instance == nullptr)
            {
                log_error("core", "factory constructor for plugin '{}' returned a nullptr", plugin_name);
                return false;
            }

            /* solve dependencies */
            std::set<std::string> dep_file_name = instance->get_dependencies();
            if (!solve_dependencies(plugin_name, dep_file_name))
            {
                return false;
            }

            /* add cli options */
            CliExtensionInterface* ceif = instance->get_first_extension<CliExtensionInterface>();
            if (ceif)
            {
                BasePluginInterface* plugin = instance.get();
                if (plugin == nullptr)
                {
                    return false;
                }

                auto cli_options = ceif->get_cli_options();
                UIPluginInterface* ui_plugin = dynamic_cast<UIPluginInterface*>(plugin);
                for (const auto& cli_option : cli_options.get_options())
                {
                    for (const auto& flag : std::get<0>(cli_option))
                    {
                        if (m_existing_options.is_registered(flag))
                        {
                            log_error("core", "command line option '{}' is already used by generic program options -- use another one.", flag);
                            return false;
                        }

                        for (int iplugType = 0; iplugType<2; iplugType++)
                            if (m_cli_option_to_plugin_name[iplugType].find(flag) != m_cli_option_to_plugin_name[iplugType].end())
                            {
                                log_error("core", "command line option '{}' is already used by plugin '{}' -- use another option in plugin '{}'.", flag, m_cli_option_to_plugin_name[iplugType][flag], plugin_name);
                            return false;
                        }
                        m_cli_option_to_plugin_name[ui_plugin?1:0][flag] = plugin_name;
                        log_debug("core", "registered command line option '{}' for plugin '{}'.", flag, plugin->get_name());
                    }
                }
                m_plugin_options.add(cli_options);
            }

            instance->initialize_logging();

            m_current_loading = plugin_name;
            instance->on_load();

            for (AbstractExtensionInterface* aeif : instance->get_extensions())
            {
                FacExtensionInterface* feif = dynamic_cast<FacExtensionInterface*>(aeif);
                if (!feif) continue;
                if (feif->get_feature() != FacExtensionInterface::FacUnknown)
                    m_plugin_features[plugin_name].push_back({feif->get_feature(),
                                                              feif->get_supported_file_extensions(),
                                                              feif->get_description()});
                switch (feif->get_feature())
                {
                case FacExtensionInterface::FacNetlistParser:
                {
                    FacFactoryProvider<NetlistParser>* fac = static_cast<FacFactoryProvider<NetlistParser>*>(feif->factory_provider);
                    netlist_parser_manager::register_parser(feif->get_description(), fac->m_factory, feif->get_supported_file_extensions());
                    break;
                }
                case FacExtensionInterface::FacNetlistWriter:
                {
                    FacFactoryProvider<NetlistWriter>* fac = static_cast<FacFactoryProvider<NetlistWriter>*>(feif->factory_provider);
                    netlist_writer_manager::register_writer(feif->get_description(), fac->m_factory, feif->get_supported_file_extensions());
                    break;
                }
                case FacExtensionInterface::FacGatelibParser:
                {
                    FacFactoryProvider<GateLibraryParser>* fac = static_cast<FacFactoryProvider<GateLibraryParser>*>(feif->factory_provider);
                    gate_library_parser_manager::register_parser(feif->get_description(), fac->m_factory, feif->get_supported_file_extensions());
                    break;
                }
                case FacExtensionInterface::FacGatelibWriter:
                {
                    FacFactoryProvider<GateLibraryWriter>* fac = static_cast<FacFactoryProvider<GateLibraryWriter>*>(feif->factory_provider);
                    gate_library_writer_manager::register_writer(feif->get_description(), fac->m_factory, feif->get_supported_file_extensions());
                    break;
                }
                default:
                    break;
                }
            }
            m_current_loading.clear();

            m_loaded_plugins[plugin_name] = std::make_tuple(std::move(instance), std::move(lib));

            /* notify callback that a plugin was loaded*/
            m_hook(true, plugin_name, file_path.string());

            log_debug("core", "loaded plugin '{}'.", plugin_name);
            return true;
        }

        bool unload_all_plugins()
        {
            if (m_loaded_plugins.size() == 0)
            {
                log_debug("core", "no plugins to unload");
                return true;
            }
            auto loaded_plugin_names = get_plugin_names();
            for (const auto& name : loaded_plugin_names)
            {
                if (!unload(name))
                {
                    log_error("core", "could not unload plugin '{}'", name);
                }
            }
            log_info("core", "unloaded all {} plugins", loaded_plugin_names.size());
            return true;
        }

        bool unload(const std::string& plugin_name)
        {
            auto it = m_loaded_plugins.find(plugin_name);
            if (it == m_loaded_plugins.end())
            {
                log_debug("core", "cannot find plugin '{}' to unload it", plugin_name);
                return true;
            }

            log_info("core", "unloading plugin '{}'...", plugin_name);


            auto rt_library  = std::move(std::get<1>(it->second));
            auto plugin_inst = std::move(std::get<0>(it->second));

            {
                auto iplugType = dynamic_cast<UIPluginInterface*>(plugin_inst.get()) ? 1 : 0;
                auto it = m_cli_option_to_plugin_name[iplugType].begin();
                while (it != m_cli_option_to_plugin_name[iplugType].end())
                {
                    auto flag = it->first;
                    auto name = it->second;
                    if (name == plugin_name)
                    {
                        m_plugin_options.remove(flag);
                        it = m_cli_option_to_plugin_name[iplugType].erase(it);
                    }
                    else
                        ++it;
                }
            }

            m_loaded_plugins.erase(it);

            for (AbstractExtensionInterface* aeif : plugin_inst->get_extensions())
            {
                FacExtensionInterface* feif = dynamic_cast<FacExtensionInterface*>(aeif);
                if (!feif) continue;
                switch (feif->get_feature())
                {
                case FacExtensionInterface::FacNetlistParser:
                    netlist_parser_manager::unregister_parser(feif->get_description());
                    break;
                case FacExtensionInterface::FacNetlistWriter:
                    netlist_writer_manager::unregister_writer(feif->get_description());
                    break;
                case FacExtensionInterface::FacGatelibParser:
                    gate_library_parser_manager::unregister_parser(feif->get_description());
                    break;
                case FacExtensionInterface::FacGatelibWriter:
                    gate_library_writer_manager::unregister_writer(feif->get_description());
                    break;
                default:
                    break;
                }
            }

            auto file_name = rt_library->get_file_name();
            plugin_inst->on_unload();

            // actual unloading
            // order of unloading is important, so we do it manually here:
            // first destroy the plugin instance, then destroy the runtime library
            plugin_inst.reset();
            rt_library.reset();

            /* notify callback that a plugin was unloaded */
            m_hook(false, plugin_name, file_name);

            log_debug("core", "unloaded plugin '{}'", plugin_name);
            return true;
        }

        BasePluginInterface* get_plugin_instance(const std::string& plugin_name, bool initialize, bool silent)
        {
            auto it = m_loaded_plugins.find(plugin_name);
            if (it == m_loaded_plugins.end())
            {
                if (!silent)
                    log_error("core", "plugin '{}' is not loaded", plugin_name);
                return nullptr;
            }

            auto instance = std::get<0>(it->second).get();
            if (instance != nullptr && initialize)
            {
                instance->initialize();
            }
            return instance;
        }

        u64 add_model_changed_callback(std::function<void(bool, std::string const&, std::string const&)> callback)
        {
            if (callback == nullptr)
            {
                log_error("core", "parameter 'callback' is nullptr");
                return 0;
            }
            return m_hook.add_callback(callback);
        }

        void remove_model_changed_callback(u64 id)
        {
            m_hook.remove_callback(id);
        }

        void add_existing_options_description(const ProgramOptions& existing_options)
        {
            m_existing_options.add(existing_options);
        }
    }    // namespace plugin_manager
}    // namespace hal
