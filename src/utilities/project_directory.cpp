#include "hal_core/utilities/project_directory.h"
#include <sstream>
#include <iostream>
#include <time.h>

namespace hal {
    const std::string ProjectDirectory::s_shadow_dir = "autosave";

    ProjectDirectory::ProjectDirectory(const std::string& dirname)
        : std::filesystem::path(dirname)
    {
        replace_extension(); // remove any extension
    }

    ProjectDirectory ProjectDirectory::generateRandom()
    {
        srand(time(nullptr));

        for (int iloop=0; iloop<1000; iloop++)
        {
            std::filesystem::path proj = std::filesystem::current_path();
            std::ostringstream oss;
            oss << "hal_proj" << (1000 + rand() % 9000);
            proj.append(oss.str());
            if (!std::filesystem::exists(proj))
                return ProjectDirectory(proj.string());
        }
        return ProjectDirectory();
    }

    std::filesystem::path ProjectDirectory::get_default_filename(const std::string& extension) const
    {
        std::string name(filename());
        name += extension.empty() ?  std::string(".hal") : extension;
        std::filesystem::path retval(*this);
        retval.append(name);
        return retval;
    }

    std::filesystem::path ProjectDirectory::get_filename(const std::string &relative_filename) const
    {
        std::filesystem::path rel(relative_filename);
        std::filesystem::path retval(*this);
        if (rel.is_relative())
            retval.append(relative_filename);
        else
            retval.append(rel.filename().string());
        return retval;
    }

    std::filesystem::path ProjectDirectory::get_shadow_filename(const std::string& extension) const
    {
        std::string name(filename());
        name += extension.empty() ?  std::string(".hal") : extension;
        std::filesystem::path retval = get_shadow_dir();
        retval.append(name);
        return retval;
    }

    std::filesystem::path ProjectDirectory::get_shadow_dir() const
    {
        std::filesystem::path retval(*this);
        retval.append(s_shadow_dir);
        return retval;
    }
}
