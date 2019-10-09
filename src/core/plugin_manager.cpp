#include "core/plugin_manager.h"

#include "core/interface_cli.h"
#include "core/log.h"
#include "core/utils.h"

#include <vector>

#ifdef _WIN32
#include <tchar.h>
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

namespace plugin_manager
{
    namespace
    {
        // stores library and factory identified by plugin name)
        std::map<std::string, std::tuple<library_loader*, i_factory*>> m_plugin;

        // stores the CLI parser option for each plugin
        std::map<std::string, std::string> m_cli_option_to_plugin_name;

        // stores the GUI callback
        callback_hook<void(bool, std::string const&, std::string const&)> m_hook;

        // stores the generic option parser
        program_options m_existing_options("existing options");

        // stores the plugin option description
        program_options m_plugin_options("plugin options");

        // stores the plugin folder
        std::vector<hal::path> m_plugin_folders = core_utils::get_plugin_directories();

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
                auto dep_plugin_name        = hal::path(file_name).stem().string();
                auto dep_file_name_with_ext = file_name + "." + LIBRARY_FILE_EXTENSION;

                if (m_plugin.find(dep_plugin_name) != m_plugin.end())
                    continue;
                hal::path file_path = core_utils::get_file(dep_file_name_with_ext, m_plugin_folders);
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

        bool has_valid_file_extension(hal::path file_name)
        {
#if defined(__APPLE__) && defined(__MACH__)
            if (core_utils::ends_with(file_name.string(), ".so"))
                return true;
            if (file_name.string().find(".so") != std::string::npos)
                return true;
            if (core_utils::ends_with(file_name.string(), ".icloud"))
                return false;
#endif
            return (core_utils::ends_with(file_name.string(), std::string(".") + std::string(LIBRARY_FILE_EXTENSION)));
        }
    }    // namespace

    std::set<std::string> get_plugin_names()
    {
        std::set<std::string> names;
        for (const auto& it : m_plugin)
        {
            names.insert(it.first);
        }
        return names;
    }

    std::map<std::string, std::string> get_flag_to_plugin_mapping()
    {
        return m_cli_option_to_plugin_name;
    }

    program_options get_cli_plugin_options()
    {
        return m_plugin_options;
    }

    bool load_all_plugins(const std::vector<hal::path>& directory_names)
    {
        auto directories = (!directory_names.empty()) ? directory_names : m_plugin_folders;
        for (const auto& directory : directories)
        {
            if (!hal::fs::exists(directory))
                continue;
            u32 num_of_loaded_plugins = 0;
            for (const auto& file : core_utils::directory_range(directory))
            {
                if (!has_valid_file_extension(file) || hal::fs::is_directory(file))
                    continue;

                auto plugin_name = file.path().stem().string();
                if (load(plugin_name, file.path()))
                    num_of_loaded_plugins++;
            }
            log_debug("core", "loaded {} plugins from '{}'", num_of_loaded_plugins, directory.string());
        }
        return true;
    }

    bool load(const std::string& plugin_name, const hal::path& file_name)
    {
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

        if (m_plugin.find(plugin_name) != m_plugin.end())
        {
            log_debug("core", "plugin '{}' is already loaded", plugin_name);
            return true;
        }

        /* load library */
        library_loader* lib = new library_loader();
        if (!lib->load_library(file_name.string()))
            return false;

        /* get factory of library */
        auto get_factory_fn = (get_factory_fn_ptr_t)lib->get_function("get_factory");
        if (get_factory_fn == nullptr)
            return false;
        auto factory = get_factory_fn();
        if (factory == nullptr)
        {
            log_error("core", "factory constructor for plugin '{}' returned a nullptr", plugin_name);
            return false;
        }

        /* solve dependencies */
        std::set<std::string> dep_file_name = factory->get_dependencies();
        if (!solve_dependencies(plugin_name, dep_file_name))
            return false;

        /* add cli options */
        auto types = factory->get_plugin_types();
        if (types.find(interface_type::cli) != types.end())
        {
            auto plugin = std::dynamic_pointer_cast<i_cli>(factory->query_interface(interface_type::cli));
            if (plugin == nullptr)
                return false;

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
                    if (m_cli_option_to_plugin_name.find(flag) != m_cli_option_to_plugin_name.end())
                    {
                        log_error("core", "command line option '{}' is already used by plugin '{}' -- use another option in plugin '{}'.", flag, m_cli_option_to_plugin_name[flag], plugin_name);
                        return false;
                    }
                    m_cli_option_to_plugin_name[flag] = plugin_name;
                    log_debug("core", "registered command line option '{}' for plugin '{}'.", flag, plugin->get_name());
                }
            }
            m_plugin_options.add(cli_options);
        }
        m_plugin[plugin_name] = std::make_tuple(lib, factory);

        auto base_plugin = factory->query_interface(interface_type::base);
        base_plugin->initialize_logging();

        base_plugin->on_load();

        /* notify callback that a plugin was loaded*/
        m_hook(true, plugin_name, file_name.string());

        log_debug("core", "loaded plugin '{}'.", plugin_name);
        return true;
    }

    bool unload_all_plugins()
    {
        /* fix start */
        /* remove all cli options */
        m_plugin_options = program_options("plugin options");
        m_cli_option_to_plugin_name = std::map<std::string, std::string>();
        /* fix end */
        if (m_plugin.size() == 0)
        {
            log_debug("core", "no plugins to unload");
            return true;
        }
        auto tmp_m_plugin = m_plugin;
        for (const auto& it : tmp_m_plugin)
        {
            if (!unload(it.first))
                return false;
        }
        log_info("core", "unloaded all {} plugins", (int)tmp_m_plugin.size());
        return true;
    }

    bool unload(const std::string& plugin_name)
    {
        auto it = m_plugin.find(plugin_name);
        if (it == m_plugin.end())
        {
            log_debug("core", "cannot find plugin '{}' to unload it", plugin_name);
            return true;
        }

        /* store file name for callback notification */
        auto file_name = std::get<0>(it->second)->get_file_name();

        /* unload and delete factory */
        auto factory = std::get<1>(it->second);
        factory->query_interface(interface_type::base)->on_unload();

        //delete factory; <- fix
        /* unload and delete library */
        auto library = std::get<0>(it->second);
        if (!library->unload_library())
            return false;
        delete library;

        /* remove plugin */
        m_plugin.erase(it);

        /* notify callback that a plugin was unloaded */
        m_hook(false, plugin_name, file_name);
        /* fix start */
        
        /* fix end */

        log_debug("core", "unloaded plugin '{}'", plugin_name);
        return true;
    }

    i_factory* get_plugin_factory(const std::string& plugin_name)
    {
        if (m_plugin.find(plugin_name) == m_plugin.end())
        {
            log_error("core", "plugin '{}' is not loaded", plugin_name);
            return nullptr;
        }
        return std::get<1>(m_plugin[plugin_name]);
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

    void add_existing_options_description(const program_options& existing_options)
    {
        m_existing_options.add(existing_options);
    }
    
    
}    // namespace plugin_manager
