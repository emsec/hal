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
#include "hal_core/netlist/project_serializer.h"
#include <QColor>
#include <QObject>
#include <QMap>

namespace hal
{
    /**
     * Manages the module colors which will be assigned either by an "random" algorithm or manually by user.
     * Quotes around "random" indicate it is not random at all, it is some homemade spinning around in HSV color circle.
     */
    class ModuleColorManager : public QObject
    {
        Q_OBJECT

    Q_SIGNALS:
        /**
         * Q_SIGNAL to notify that the color of a module has been changed.
         *
         * @param id - Id of the module with the changed color
         */
        void moduleColorChanged(u32 id) const;

    public:
        ModuleColorManager(QObject* parent = nullptr);

        /**
         * Gets the module color of a module of a specific id.
         *
         * @param id - The module id of the module to get the color for
         * @returns the color of the module
         */
        QColor moduleColor(u32 id) const;

        /**
         * Changes the color of a module.
         *
         * @param id - The id of the module
         * @param col - The new color
         * @returns the old color of the module (used to create an undo action easier)
         */
        QColor setModuleColor(u32 id, const QColor& col);

        /**
         * Changes the color of a module to a random color.
         *
         * @param id - The id of the module
         * @returns the old color of the module (used to create an undo action easier)
         */
        QColor setRandomColor(u32 id);

        /**
         * Removes the color that belongs to the given id.
         *
         * @param id - The module id for which to remove the color.
         */
        void removeColor(u32 id);

        /**
         * Getter for map of all module colores
         *
         * @returns The map of all module colores
         */
        QMap<u32, QColor> getColorMap() const;

        void clear();
    private:


        /**
         * Returns a somewhat random color through a funny method (should be the same order
         * of colors each time the program starts).
         *
         * @return The "random" color.
         */
        QColor getRandomColor();

        QMap<u32, QColor> mModuleColors;
    };

    /**
     * Persist module color settings to file and restore module color assignment from previous session
     */
    class ModuleColorSerializer : public ProjectSerializer
    {
        void restoreModuleColor(const std::filesystem::path& loaddir, const std::string& jsonfile, ModuleColorManager *mcm = nullptr);
        void serializeColorMap(QJsonArray& mcArr, const ModuleColorManager* mcm);
    public:
        ModuleColorSerializer();

        std::string serialize(Netlist* netlist, const std::filesystem::path& savedir, bool isAutosave) override;

        void deserialize(Netlist* netlist, const std::filesystem::path& loaddir) override;

        void restore(ModuleColorManager *mcm);
    };
}
