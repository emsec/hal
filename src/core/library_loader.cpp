#include "core/library_loader.h"

#include "core/log.h"

namespace hal
{
    LibraryLoader::LibraryLoader()
    {
        m_handle = NULL;
    }

    LibraryLoader::~LibraryLoader()
    {
        if (m_handle != NULL)
        {
            this->unload_library();
        }
        m_file_name.clear();
    }

    std::string LibraryLoader::get_file_name() const
    {
        return m_file_name;
    }

    bool LibraryLoader::load_library(const std::string& file_name)
    {
#ifdef _WIN32
        m_handle = LoadLibrary(file_name.c_str());
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
        error_message = "to be implemented for windows based operating systems.";
#else
        error_message = dlerror();
#endif
        log_error("core", "cannot load library '{}' (error: {}) -- did you properly link the plugin?", file_name, error_message);
        return false;
    }

    bool LibraryLoader::unload_library()
    {
        if (m_handle == NULL)
        {
            log_error("core", "library '{}' already unloaded", m_file_name);
            return true;
        }
#ifdef _WIN32
        if (FreeLibrary(m_handle) == 0)
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

    lib_fn_ptr_t LibraryLoader::get_function(const std::string& function_name)
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
