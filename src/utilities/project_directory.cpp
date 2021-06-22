#include "hal_core/utilities/project_directory.h"
#include <sstream>
#include <iostream>
#include <time.h>

namespace hal {
    ProjectDirectory::ProjectDirectory(const std::string& dirname)
        : std::filesystem::path(dirname)
    {
        if (dirname.empty())
        {
            srand(time(nullptr));
            std::filesystem::path cur = std::filesystem::current_path();
            std::ostringstream oss;
            oss << "hal_proj" << (1000 + rand() % 9000);
            cur.append(oss.str());
            assign(cur);
        }
        else
            replace_extension(); // remove any extension
    }

    std::filesystem::path ProjectDirectory::get_filename(bool relative, const std::string& extension) const
    {
        std::string name(filename());
        name += extension.empty() ?  std::string(".hal") : extension;
        if (relative) return std::filesystem::path(name);
        std::filesystem::path retval(*this);
        retval.append(name);
        return retval;
    }
}
