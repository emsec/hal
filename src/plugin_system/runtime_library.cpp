#include "hal_core/plugin_system/runtime_library.h"

#include <filesystem>

#include "hal_core/utilities/log.h"

namespace hal
{
    RuntimeLibrary::RuntimeLibrary()
    {
        m_handle = NULL;
    }

    RuntimeLibrary::~RuntimeLibrary()
    {
        if (m_handle != NULL)
        {
            this->unload_library();
        }
        m_file_name.clear();
    }

    std::string RuntimeLibrary::get_file_name() const
    {
        return m_file_name;
    }

    bool RuntimeLibrary::load_library(const std::string& file_name)
    {
#ifdef _WIN32
        // plugins link against each other, so the directory of the plugin being loaded has to be
        // searched for its dependencies as well. SetDllDirectory adds it to the standard search
        // order, which keeps both the directory of hal itself and PATH in the search.
        const auto native_path = std::filesystem::absolute(std::filesystem::path(file_name)).make_preferred();
        SetDllDirectoryA(native_path.parent_path().string().c_str());
        m_handle = LoadLibraryA(native_path.string().c_str());
        SetDllDirectoryA(NULL);
#else
        m_handle      = dlopen((file_name).c_str(), RTLD_GLOBAL | RTLD_LAZY);
#endif

        if (m_handle != NULL)
        {
            log_debug("core", "loaded library '{}'", file_name);
            m_file_name = file_name;
            return true;
        }

        auto error_message = std::string("");
#ifdef _WIN32
        char* buffer = NULL;
        const auto length =
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), 0, (char*)&buffer, 0, NULL);
        if (length != 0)
        {
            error_message = std::string(buffer, length);
            while (!error_message.empty() && (error_message.back() == '\n' || error_message.back() == '\r'))
            {
                error_message.pop_back();
            }
        }
        LocalFree(buffer);
#else
        error_message = dlerror();
#endif
        log_error("core", "cannot load library '{}' (error: {}) -- did you properly link the plugin?", file_name, error_message);
        return false;
    }

    bool RuntimeLibrary::unload_library()
    {
        if (m_handle == NULL)
        {
            log_error("core", "library '{}' already unloaded", m_file_name);
            return true;
        }
#ifdef _WIN32
        // FreeLibrary reports success with a non-zero value, unlike dlclose
        if (FreeLibrary(m_handle) != 0)
        {
            log_debug("core", "unloaded library '{}'", m_file_name);
            m_handle = NULL;
            return true;
        }
#else
        if (dlclose(m_handle) == 0)
        {
            log_debug("core", "unloaded library '{}'", m_file_name);
            m_handle = NULL;
            return true;
        }
#endif

        log_error("core", "cannot unload library '{}'", m_file_name);
        return false;
    }

    lib_fn_ptr_t RuntimeLibrary::get_function(const std::string& function_name)
    {
        lib_fn_ptr_t fptr = NULL;
#ifdef _WIN32
        fptr = GetProcAddress(m_handle, function_name.c_str());
        if (!fptr)
        {
            log_error("core", "cannot load function '{}' from library '{}'", function_name, m_file_name);
            return NULL;
        }
#else
        dlerror();
        fptr = (lib_fn_ptr_t)dlsym(m_handle, function_name.c_str());
        if (dlerror())
        {
            log_debug("core", "cannot load function '{}' from library '{}'", function_name, m_file_name);
            return NULL;
        }
#endif
        return fptr;
    }
}    // namespace hal
