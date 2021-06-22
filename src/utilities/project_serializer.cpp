#include "hal_core/utilities/project_serializer.h"
#include "hal_core/utilities/project_manager.h"

namespace hal {
    ProjectSerializer::ProjectSerializer(const std::string& name)
        : m_name(name)
    {
        ProjectManager::instance()->register_serializer(m_name, this);
    }
}
