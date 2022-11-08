#include "hal_core/utilities/project_directory.h"
#include <sstream>
#include <iostream>
#include <time.h>

namespace hal {
    const std::string ProjectDirectory::s_shadow_dir = "autosave";

    ProjectDirectory::ProjectDirectory(const std::string& path_)
        : std::filesystem::path(path_)
    {
        replace_extension(); // remove any extension
    }

    ProjectDirectory ProjectDirectory::generate_random()
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

    std::filesystem::path ProjectDirectory::get_canonical_path() const
    {
        if (empty()) return std::filesystem::path();
        if (!std::filesystem::exists(*this)) return std::filesystem::path();
        return std::filesystem::canonical(*this);
    }

    std::filesystem::path ProjectDirectory::get_default_filename(const std::string& extension) const
    {
        std::string name(filename());
        name += extension.empty() ?  std::string(".hal") : extension;
        std::filesystem::path retval(get_canonical_path());
        retval.append(name);
        return retval;
    }

    std::filesystem::path ProjectDirectory::get_filename(const std::string &relative_filename) const
    {
        std::filesystem::path rel(relative_filename);
        std::filesystem::path retval(get_canonical_path());
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
        std::filesystem::path retval(get_canonical_path());
        retval.append(s_shadow_dir);
        return retval;
    }

    std::filesystem::path ProjectDirectory::get_relative_file_path(const std::string &filename) const
    {
        std::filesystem::path retval(filename);
        if (retval.is_relative() || filename.empty() || empty()) return retval;
        std::string relative = filename;
        std::string canonical_dir = get_canonical_path().string() + '/';
        int n = canonical_dir.size();
        if (!relative.compare(0,n,canonical_dir))
            return (std::filesystem::path(relative.substr(n)));
        return retval;
    }
}
