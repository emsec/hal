// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "hal_core/defines.h"
#include <filesystem>
#include <string>

namespace hal {

    class Netlist;

    class ProjectSerializer
    {
    protected:
        std::string m_name;

    public:
        /**
         * Constructor
         * @param name Tag name for serializer to be identified by project manager
         */
        ProjectSerializer(const std::string& name);

        /**
         * Virtual destructor
         */
        virtual ~ProjectSerializer();

        /**
         * Abstract serialize method to be implemented by derived class. If there was
         * no need for serialization or serialization failed an empty file name should
         * be returned.
         * @param[in] netlist The netlist to be serialized
         * @param[in] savedir Target project directory
         * @param[in] isAutosave True if serialize call was issued from autosave, false otherwise
         * @return relative name of file name that was used for serialization
         */
        virtual std::string serialize(Netlist* netlist, const std::filesystem::path& savedir, bool isAutosave) = 0;

        /**
         * Abstract deserialize method to be implemented by derived class. Note that
         * it can be assumed upon calling this method that the deserialization of the
         * netlist has been successful. Caller will provide project directory name
         * but not the filename. The latter one can be queried from ProjectManager.
         * @param[in] netlist The netlist already deserialized
         * @param[in] loaddir Project directory to load files from
         */
        virtual void deserialize(Netlist* netlist, const std::filesystem::path& loaddir) = 0;
   };
}
