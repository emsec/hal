#include "hal_core/netlist/project_serializer.h"
#include "hal_core/netlist/project_manager.h"

namespace hal {
    ProjectSerializer::ProjectSerializer(const std::string& name)
        : m_name(name)
    {
        ProjectManager::instance()->register_serializer(m_name, this);
    }

    ProjectSerializer::~ProjectSerializer()
    {
        ProjectManager::instance()->unregister_serializer(m_name);
    }
}


