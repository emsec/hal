//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "hal_core/defines.h"
#include "hal_core/plugin_system/plugin_interface_base.h"
#include "hal_core/netlist/project_serializer.h"
#include <QDir>

namespace hal
{
    class Netlist;
    class NetlistSimulatorController;
    class NetlistSimulator;
    class SimulationSettings;
    class Netlist;

    class SimulatorSerializer : public ProjectSerializer
    {
        Netlist* mNetlist;
        QDir mProjDir;
     //   void restoreSimulator(const std::filesystem::path& loaddir, const std::string& jsonfile);
    public:
        SimulatorSerializer();

        std::string serialize(Netlist* netlist, const std::filesystem::path& savedir, bool isAutosave) override;

        void deserialize(Netlist* netlist, const std::filesystem::path& loaddir) override;

        std::vector<std::unique_ptr<NetlistSimulatorController>> restore();
    };

    class PLUGIN_API NetlistSimulatorControllerPlugin : public BasePluginInterface
    {
        static u32 sMaxControllerId;
    public:
        /**
         * Get the name of the plugin.
         *
         * @returns The name of the plugin.
         */
        std::string get_name() const override;

        /**
         * Get the version of the plugin.
         *
         * @returns The version of the plugin.
         */
        std::string get_version() const override;

        /**
         * Delete all bindings upon unload
         */
        void on_unload() override;

        /**
         * Register resources upon load
         */
        void on_load() override;

        /**
         * Create a netlist simulator controller instance.
         *
         * @returns The simulator instance.
         */
        std::unique_ptr<NetlistSimulatorController> create_simulator_controller(const std::string& nam=std::string(), const std::string& workdir=std::string()) const;

        /**
         * Restore controller with waveform data from previous simulation.
         * @param[in] nl The netlist the simulation was performed with.
         * @param[in] filename Full path and filename of 'netlist_simulator_controller.json'.
         * @param[in] workdir Working directory will be created at given location.
         *
         * @returns The simulator instance.
         */
        std::unique_ptr<NetlistSimulatorController> restore_simulator_controller(Netlist* nl, const std::string& filename) const;

        /**
         * Share netlist simulator controller instance addressed by id
         * @param[in] id Controller-Id
         * @return Shared pointer to simulator
         */
        std::shared_ptr<NetlistSimulatorController> simulator_controller_by_id(u32 id) const;

        /**
         * Pointer to simulation controller settings. Use sync() to persist settings.
         */
        static SimulationSettings* sSimulationSettings;
        static SimulatorSerializer* sSimulatorSerializer;
    };
}    // namespace hal
