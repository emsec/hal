#include "hal_core/plugin_system/plugin_manager.h"

#include "hal_core/utilities/log.h"
#include "hal_core/plugin_system/plugin_interface_cli.h"
#include "hal_core/plugin_system/plugin_interface_ui.h"
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
            std::unordered_map<std::string, std::tuple<LibraryLoader*, std::unique_ptr<BasePluginInterface>>> m_loaded_plugins;

            // stores the CLI parser option for CLI plugins
            std::unordered_map<std::string, std::string> m_cli_option_to_cli_plugin_name;

            // stores the CLI parser option for UI plugins
            std::unordered_map<std::string, std::string> m_cli_option_to_ui_plugin_name;

            // stores the GUI callback
            CallbackHook<void(bool, std::string const&, std::string const&)> m_hook;

            // stores the generic option parser
            ProgramOptions m_existing_options("existing options");

            // stores the plugin option description
            ProgramOptions m_plugin_options("plugin options");

            // stores the plugin folder
            std::vector<std::filesystem::path> m_plugin_folders = utils::get_plugin_directories();

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
                    auto dep_file_name_with_ext = file_name + "." + LIBRARY_FILE_EXTENSION;

                    if (m_loaded_plugins.find(dep_plugin_name) != m_loaded_plugins.end())
                    {
                        continue;
                    }
                    std::filesystem::path file_path = utils::get_file(dep_file_name_with_ext, m_plugin_folders);
                    if (file_path == "" || !load(dep_plugin_name, file_path))
                    {
                        log_error("core", "cannot solve dependency '{}' for plugin '{}'", dep_plugin_name, plugin_name);
                        return false;
                    }

                    log_debug("core", "solved dependency '{}' for plugin '{}'", dep_plugin_name, plugin_name);
                }
                log_debug("core", "solved {} dependencies for plugin '{}'", dep_file_name.size(), plugin_name);
                return true;
            }

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
                return (utils::ends_with(file_name.string(), std::string(".") + std::string(LIBRARY_FILE_EXTENSION)));
            }
        }    // namespace

        std::set<std::string> get_plugin_names()
        {
            std::set<std::string> names;
            for (const auto& it : m_loaded_plugins)
            {
                names.insert(it.first);
            }
            return names;
        }

        std::unordered_map<std::string, std::string> get_cli_plugin_flags()
        {
            return m_cli_option_to_cli_plugin_name;
        }

        std::unordered_map<std::string, std::string> get_ui_plugin_flags()
        {
            return m_cli_option_to_ui_plugin_name;
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

        bool load(const std::string& plugin_name, const std::filesystem::path& file_name)
        {
            log_info("core", "loading plugin '{}'...", file_name.string());
            if (plugin_name.empty())
            {
                log_error("core", "parameter 'plugin_name' is empty");
                return false;
            }
            if (file_name.empty())
            {
                log_error("core", "parameter 'file_name' is empty");
                return false;
            }

            if (m_loaded_plugins.find(plugin_name) != m_loaded_plugins.end())
            {
                log_debug("core", "plugin '{}' is already loaded", plugin_name);
                return true;
            }

            /* load library */
            LibraryLoader* lib = new LibraryLoader();
            if (!lib->load_library(file_name.string()))
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
            if (instance->has_type(PluginInterfaceType::cli))
            {
                auto plugin = dynamic_cast<CLIPluginInterface*>(instance.get());
                if (plugin == nullptr)
                {
                    return false;
                }

                auto cli_options = plugin->get_cli_options();
                for (const auto& cli_option : cli_options.get_options())
                {
                    for (const auto& flag : std::get<0>(cli_option))
                    {
                        if (m_existing_options.is_registered(flag))
                        {
                            log_error("core", "command line option '{}' is already used by generic program options -- use another one.", flag);
                            return false;
                        }
                        if (m_cli_option_to_cli_plugin_name.find(flag) != m_cli_option_to_cli_plugin_name.end())
                        {
                            log_error(
                                "core", "command line option '{}' is already used by plugin '{}' -- use another option in plugin '{}'.", flag, m_cli_option_to_cli_plugin_name[flag], plugin_name);
                            return false;
                        }
                        m_cli_option_to_cli_plugin_name[flag] = plugin_name;
                        log_debug("core", "registered command line option '{}' for plugin '{}'.", flag, plugin->get_name());
                    }
                }
                m_plugin_options.add(cli_options);
            }
            if (instance->has_type(PluginInterfaceType::interactive_ui))
            {
                auto plugin = dynamic_cast<UIPluginInterface*>(instance.get());
                if (plugin == nullptr)
                {
                    return false;
                }

                auto cli_options = plugin->get_cli_options();
                for (const auto& cli_option : cli_options.get_options())
                {
                    for (const auto& flag : std::get<0>(cli_option))
                    {
                        if (m_existing_options.is_registered(flag))
                        {
                            log_error("core", "command line option '{}' is already used by generic program options -- use another one.", flag);
                            return false;
                        }
                        if (m_cli_option_to_cli_plugin_name.find(flag) != m_cli_option_to_cli_plugin_name.end())
                        {
                            log_error(
                                "core", "command line option '{}' is already used by cli plugin '{}' -- use another option in plugin '{}'.", flag, m_cli_option_to_cli_plugin_name[flag], plugin_name);
                            return false;
                        }
                        if (m_cli_option_to_ui_plugin_name.find(flag) != m_cli_option_to_ui_plugin_name.end())
                        {
                            log_error(
                                "core", "command line option '{}' is already used by ui plugin '{}' -- use another option in plugin '{}'.", flag, m_cli_option_to_ui_plugin_name[flag], plugin_name);
                            return false;
                        }
                        m_cli_option_to_ui_plugin_name[flag] = plugin_name;
                        log_debug("core", "registered command line option '{}' for plugin '{}'.", flag, plugin->get_name());
                    }
                }
                m_plugin_options.add(cli_options);
            }

            instance->initialize_logging();

            instance->on_load();

            m_loaded_plugins[plugin_name] = std::make_tuple(lib, std::move(instance));

            /* notify callback that a plugin was loaded*/
            m_hook(true, plugin_name, file_name.string());

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

            {
                auto tmp = m_cli_option_to_cli_plugin_name;
                for (auto it : tmp)
                {
                    if (it.second == plugin_name)
                    {
                        m_cli_option_to_cli_plugin_name.erase(it.first);
                    }
                }
            }
            {
                auto tmp = m_cli_option_to_ui_plugin_name;
                for (auto it : tmp)
                {
                    if (it.second == plugin_name)
                    {
                        m_cli_option_to_ui_plugin_name.erase(it.first);
                    }
                }
            }

            /* unload */
            auto library = std::get<0>(it->second);
            std::get<1>(it->second)->on_unload();

            /* remove plugin */
            m_loaded_plugins.erase(it);

            /* unload and delete library */
            auto file_name = library->get_file_name();
            if (!library->unload_library())
            {
                return false;
            }
            delete library;

            /* notify callback that a plugin was unloaded */
            m_hook(false, plugin_name, file_name);

            log_debug("core", "unloaded plugin '{}'", plugin_name);
            return true;
        }

        BasePluginInterface* get_plugin_instance(const std::string& plugin_name, bool initialize)
        {
            auto it = m_loaded_plugins.find(plugin_name);
            if (it == m_loaded_plugins.end())
            {
                log_error("core", "plugin '{}' is not loaded", plugin_name);
                return nullptr;
            }

            auto instance = std::get<1>(it->second).get();
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
