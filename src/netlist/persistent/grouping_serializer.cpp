#include "hal_core/netlist/persistent/grouping_serializer.h"
#include "hal_core/utilities/project_directory.h"
#include "hal_core/utilities/project_filelist.h"
#include "hal_core/utilities/json_write_document.h"
#include <filesystem>
#include <fstream>
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/grouping.h"

namespace hal {
    GroupingSerializer* GroupingSerializer::instance = new GroupingSerializer();

    GroupingSerializer::GroupingSerializer()
        : ProjectSerializer("grouping")
    {;}

    ProjectFilelist* GroupingSerializer::serialize(Netlist* netlist, const ProjectDirectory& haldir)
    {
        std::filesystem::path groupingFilePath = haldir.get_filename(false, ".halg");

        JsonWriteDocument doc;

        JsonWriteArray& grpArr = doc.add_array("groupings");

        for (Grouping* grp : netlist->get_groupings())
        {
            JsonWriteObject& grpObj = grpArr.add_object();
            grpObj["id"]   = grp->get_id();
            grpObj["name"] = grp->get_name();

            JsonWriteArray&  grpGates = grpObj.add_array("gates");
            std::vector<Gate*> sorted = grp->get_gates();
            std::sort(sorted.begin(), sorted.end(), [](Gate* lhs, Gate* rhs) { return lhs->get_id() < rhs->get_id(); });
            for (const Gate* g : sorted)
                grpGates << g->get_id();
            grpGates.close();


        }
        rapidjson::Value (rapidjson::kObjectType);

        val.AddMember("id", grouping->get_id(), allocator);
        val.AddMember("name", grouping->get_name(), allocator);
        {
            rapidjson::Value gates(rapidjson::kArrayType);
            std::vector<Gate*> sorted = grouping->get_gates();
            std::sort(sorted.begin(), sorted.end(), [](Gate* lhs, Gate* rhs) { return lhs->get_id() < rhs->get_id(); });
            for (auto gate : sorted)
            {
                gates.PushBack(gate->get_id(), allocator);
            }
            if (!gates.Empty())
            {
                val.AddMember("gates", gates, allocator);
            }
        }
        {
            rapidjson::Value nets(rapidjson::kArrayType);
            std::vector<Net*> sorted = grouping->get_nets();
            std::sort(sorted.begin(), sorted.end(), [](Net* lhs, Net* rhs) { return lhs->get_id() < rhs->get_id(); });
            for (auto net : sorted)
            {
                nets.PushBack(net->get_id(), allocator);
            }
            if (!nets.Empty())
            {
                val.AddMember("nets", nets, allocator);
            }
        }
        {
            rapidjson::Value modules(rapidjson::kArrayType);
            std::vector<Module*> sorted = grouping->get_modules();
            std::sort(sorted.begin(), sorted.end(), [](Module* lhs, Module* rhs) { return lhs->get_id() < rhs->get_id(); });
            for (auto module : sorted)
            {
                modules.PushBack(module->get_id(), allocator);
            }
            if (!modules.Empty())
            {
                val.AddMember("modules", modules, allocator);
            }
        }

        return val;

        doc.serialize(groupingFilePath.string());

        ProjectFilelist* retval = new ProjectFilelist;
        retval->push_back(groupingFilePath.filename().string());
        return retval;
    }

    void GroupingSerializer::deserialize(Netlist* netlist, const ProjectDirectory& haldir)
    {

    }

}
