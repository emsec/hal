#include "hal_core/netlist/persistent/grouping_serializer.h"
#include "hal_core/utilities/project_directory.h"
#include "hal_core/netlist/project_manager.h"
#include "hal_core/utilities/json_write_document.h"
#include <filesystem>
#include <fstream>
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/json_write_document.h"
#include "rapidjson/filereadstream.h"

namespace hal {
    GroupingSerializer* GroupingSerializer::instance = new GroupingSerializer();

    GroupingSerializer::GroupingSerializer()
        : ProjectSerializer("groupings")
    {;}

    std::string GroupingSerializer::serialize(Netlist* netlist, const std::filesystem::path& savedir, bool isAutosave)
    {
        std::filesystem::path groupingFilePath(savedir);
        groupingFilePath.append("groupings.json");

        JsonWriteDocument doc;

        JsonWriteArray& grpArr = doc.add_array("groupings");

        for (Grouping* grp : netlist->get_groupings())
        {
            JsonWriteObject& grpObj = grpArr.add_object();
            grpObj["id"]   = grp->get_id();
            grpObj["name"] = grp->get_name();

            JsonWriteArray& grpMod = grpObj.add_array("modules");
            std::vector<Module*> sortMod = grp->get_modules();
            std::sort(sortMod.begin(), sortMod.end(), [](Module* lhs, Module* rhs) { return lhs->get_id() < rhs->get_id(); });
            for (const Module* m : sortMod)
                grpMod << m->get_id();
            grpMod.close();

            JsonWriteArray& grpGat = grpObj.add_array("gates");
            std::vector<Gate*> sortGat = grp->get_gates();
            std::sort(sortGat.begin(), sortGat.end(), [](Gate* lhs, Gate* rhs) { return lhs->get_id() < rhs->get_id(); });
            for (const Gate* g : sortGat)
                grpGat << g->get_id();
            grpGat.close();

            JsonWriteArray& grpNet = grpObj.add_array("nets");
            std::vector<Net*> sortNet = grp->get_nets();
            std::sort(sortNet.begin(), sortNet.end(), [](Net* lhs, Net* rhs) { return lhs->get_id() < rhs->get_id(); });
            for (const Net* n : sortNet)
                grpNet << n->get_id();
            grpNet.close();

            grpObj.close();
        }

        grpArr.close();

        doc.serialize(groupingFilePath.string());

        return groupingFilePath.filename().string();
    }

    void GroupingSerializer::deserialize(Netlist* netlist, const std::filesystem::path &loaddir)
    {        
        std::string relname = ProjectManager::instance()->get_filename(m_name);
        if (relname.empty()) return;
        std::filesystem::path groupingFilePath(loaddir);
        groupingFilePath.append(relname);

        FILE* grpFile = fopen(groupingFilePath.string().c_str(), "rb");
        if (grpFile == NULL)
        {
            log_error("GroupingSerializer::deserialize", "unable to open '{}'.", groupingFilePath.string());
            return;
        }

        char buffer[65536];
        rapidjson::FileReadStream frs(grpFile, buffer, sizeof(buffer));
        rapidjson::Document document;
        document.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(frs);

        if (document.HasMember("groupings"))
        {
            for (const rapidjson::Value& grpVal : document["groupings"].GetArray())
            {
                Grouping* grouping = netlist->create_grouping(grpVal["id"].GetUint(), grpVal["name"].GetString());
                if (grouping == nullptr)
                {
                    return;
                }

                if (grpVal.HasMember("modules"))
                {
                    for (auto& module_node : grpVal["modules"].GetArray())
                    {
                        grouping->assign_module(netlist->get_module_by_id(module_node.GetUint()));
                    }
                }

                if (grpVal.HasMember("gates"))
                {
                    for (auto& gate_node : grpVal["gates"].GetArray())
                    {
                        grouping->assign_gate(netlist->get_gate_by_id(gate_node.GetUint()));
                    }
                }

                if (grpVal.HasMember("nets"))
                {
                    for (auto& net_node : grpVal["nets"].GetArray())
                    {
                        grouping->assign_net(netlist->get_net_by_id(net_node.GetUint()));
                    }
                }
            }
        }
    }
}
