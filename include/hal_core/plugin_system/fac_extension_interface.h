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
#include "hal_core/plugin_system/abstract_extension_interface.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>

namespace hal  {


    /**
     * The base class for the type-erased factory that a file-access extension hands to the framework.
     */
    class AbstractFactoryProvider
    {
    public:
        AbstractFactoryProvider() = default;
        virtual ~AbstractFactoryProvider() = default;
    };

    /**
     * Holds the factory function that creates instances of the file-access implementation `T`,
     * e.g., a netlist parser or a gate library writer contributed by a plugin.
     */
    template <typename T> class FacFactoryProvider : public AbstractFactoryProvider
    {
    public:
        FacFactoryProvider() = default;
        virtual ~FacFactoryProvider() = default;
        std::function<std::unique_ptr<T>()> m_factory;
    };

    /**
     * A plugin extension that registers a factory for a file format, e.g., a netlist parser or a gate library writer.
     * It advertises the feature it provides together with the file extensions that it supports.
     */
    class FacExtensionInterface : public AbstractExtensionInterface
    {
    public:
        enum Feature { FacUnknown, FacNetlistParser, FacNetlistWriter, FacGatelibParser, FacGatelibWriter };

    protected:
        Feature m_feature;
        std::string m_description;
        std::vector<std::string> m_supported_file_extensions;
    public:
        FacExtensionInterface(Feature feat) : m_feature(feat) {;}
        Feature get_feature() const { return m_feature; }
        std::string get_description() const { return m_description; }
        std::vector<std::string> get_supported_file_extensions() const { return m_supported_file_extensions; }
        AbstractFactoryProvider* factory_provider;
    };
}
