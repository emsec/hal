#include "netlist_modifier/netlist_modifier.h"

#include "hal_core/netlist/project_manager.h"

#include "hal_core/netlist/gate_library/gate_library_writer/gate_library_writer_manager.h"

#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>

namespace hal
{
    const char* OBFUSCATED = "_obfuscated";
    const char* GATE_LIB_TAG = "gate_library";

    extern Netlist* gNetlist;

    bool NetlistModifierPlugin::modify_gatelibrary()
    {
        ProjectManager* pm = ProjectManager::instance();

        std::filesystem::path projFilePath(pm->get_project_directory());
        if (projFilePath.empty() || !std::filesystem::exists(projFilePath))
        {
            log_warning("netlist_modifier", "Cannot access project directory '{}'.", projFilePath.string());
            return false;
        }
        projFilePath.append(ProjectManager::s_project_file);

        // get location of current gate library from project file
        FILE* fp = fopen(projFilePath.string().c_str(), "rb");
        if (fp == NULL)
        {
            log_warning("netlist_modifier", "Cannot open project file '{} for reading'.", projFilePath.string());
            return false;
        }

        char buffer[65536];
        rapidjson::FileReadStream frs(fp, buffer, sizeof(buffer));
        rapidjson::Document doc;
        doc.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(frs);
        fclose(fp);

        std::filesystem::path modifiedGateLibraryPath;
        if (doc.HasMember(GATE_LIB_TAG))
        {
            std::filesystem::path originalGateLibraryPath = doc[GATE_LIB_TAG].GetString();
            if (originalGateLibraryPath.stem().string().find(OBFUSCATED) != std::string::npos)
            {
                // using already gate library with obfuscated gates
                return true;
            }
            // generate new name for gate library with obfuscated gates
            modifiedGateLibraryPath = pm->get_project_directory().get_filename(originalGateLibraryPath.stem().string() + OBFUSCATED + ".hgl");
        }
        else
        {
            log_warning("netlist_modifier", "Cannot find mandatory '{}' tag in project file '{}'.", GATE_LIB_TAG, projFilePath.string());
            return false; // gate library entry missing in project file
        }

        // yes, we know what we are doing when casting away const ;-)
        //     Modifying attibutes of a GateType in HAL while netlist is loaded might produce unpredictable results,
        //     thus GateLibrary is declared const. However, we are only adding GateType's, this should be safe.
        GateLibrary* gl = const_cast<GateLibrary*>(gNetlist->get_gate_library());

        // map gate type categories by number of pins
        std::unordered_map<u32,int> pinCountMap;
        for (auto const& [key, gt] : gl->get_gate_types())
        {
            int count[5] = {0, 0, 0, 0, 0};
            for (const GatePin* gp : gt->get_pins())
            {
                count[(int)gp->get_direction()]++;
            }
            u32 pinCount = (count[1] & 0x3FF) | ((count[2] & 0x3FF) << 10) | ((count[3] & 0x3FF) << 20);
            ++pinCountMap[pinCount];
        }

        // create dummy gate types with appropriate number of pins
        for  (auto const& [pc,count] : pinCountMap)
        {
            int inCount  = pc & 0x3FF;
            int outCount = (pc >> 10) & 0x3FF;
            int ioCount  = (pc >> 20) & 0x3FF;
            GateType* gt = gl->create_gate_type(obfuscated_gate_name(inCount, outCount, ioCount));
            for (int i=0; i<inCount; i++)
                gt->create_pin("I" + std::to_string(i), PinDirection::input, PinType::none, true);
            for (int i=0; i<outCount; i++)
                gt->create_pin("O" + std::to_string(i), PinDirection::output, PinType::none, true);
            for (int i=0; i<ioCount; i++)
                gt->create_pin("IO" + std::to_string(i), PinDirection::inout, PinType::none, true);
        }

        if (!gate_library_writer_manager::write(gl, modifiedGateLibraryPath))
        {
            log_warning("netlist_modifier", "cannot write modified gate library to file'{}'.", modifiedGateLibraryPath.string());
            return false;
        }

        // write modified project file
        rapidjson::Value::MemberIterator gatelibMemberIterator = doc.FindMember(GATE_LIB_TAG);
        gatelibMemberIterator->value.SetString(modifiedGateLibraryPath.filename().string().c_str(), doc.GetAllocator());

        FILE* of = fopen(projFilePath.string().c_str(), "wb");
        if (of == NULL)
        {
            log_warning("netlist_modifier", "cannot open project file '{}' for rewrite.", projFilePath.string());
            return false;
        }
        rapidjson::FileWriteStream fws(of, buffer, sizeof(buffer));
        rapidjson::Writer<rapidjson::FileWriteStream> writer(fws);
        doc.Accept(writer);
        fclose(of);
        gl->set_path(modifiedGateLibraryPath);
        pm->set_gate_library_path(modifiedGateLibraryPath.filename().string());

        return true;
    }
}