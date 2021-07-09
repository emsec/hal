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

/**
 * @defgroup gui HAL Gui
 * The Gui is a huge plugin that provides a graphical user interface to load and work with Netlist%s. If compiled
 * it can be started using the '-g'/'--gui' flag. To learn more about the GUI and its structure make sure to check
 * out our [GitHub Wiki](https://github.com/emsec/hal/wiki/GUI)!
 */

    /**
     * @ingroup gui
     * @defgroup graph Graph Widget
     * Contains all classes that are relevant for the GraphWidget and the management of GraphContext%s
     */

        /**
         * @ingroup graph
         * @defgroup graph-visuals Visuals
         * Contains the classes that are responsible for building up and showing the scene. It also contains the
         * GraphicsItem%s the scene is built with.
         */

            /**
             * @ingroup graph-visuals
             * @defgroup graph-visuals-gate Graphics Gates
             * Contains all GraphicsItem%s for displaying Gate%s in the scene.
             */

            /**
             * @ingroup graph-visuals
             * @defgroup graph-visuals-module Graphics Modules
             * Contains all GraphicsItem%s for displaying Module%s in the scene.
             */

            /**
             * @ingroup graph-visuals
             * @defgroup graph-visuals-net Graphics Net
             * Contains all GraphicsItem%s for displaying Net%s in the scene.
             */

        /**
         * @ingroup graph
         * @defgroup graph-contexts Context Management
         * Contains all classes that are related to GraphContext%s.
         */

        /**
         * @ingroup graph
         * @defgroup graph-layouter Layouters
         * Contains all classes that are relevant for the layouting process.
         */

    /**
     * @ingroup gui
     * @defgroup python Python
     * Contains everything that can be associated with the python usage within the GUI.
     */

        /**
         * @ingroup python
         * @defgroup python-editor Python Editor
         * Contains all classes that are related to the PythonEditor widget.
         */

        /**
         * @ingroup python
         * @defgroup python-console Python Console
         * Contains all classes that are related to the PythonConsoleWidget.
         */

    /**
    * @ingroup gui
    * @defgroup docking Docking System
    * Contains all classes that are relevant for the ContentWidget's docking system.
    */

    /**
     * @ingroup gui
     * @defgroup logging Logging System
     * Contains everything that can be associated with the LoggerWidget.
     */

    /**
     * @ingroup gui
     * @defgroup settings Settings System
     * Contains all classes that belong to the settings system.
     */

    /**
     * @ingroup gui
     * @defgroup user_action User Actions
     * Contains the classes for all available UserAction%s and the UserAction management system.
     */

    /**
     * @ingroup gui
     * @defgroup file_management File Management
     * Contains the classes that are responsible for working with files.
     */

    /**
     * @ingroup gui
     * @defgroup utility_widgets Utility Widgets
     * Contains classes related to the ContextManagerWidget, ModuleWidget, GroupingsWidget and the SelectionDetailsWidget.
     */

        /**
        * @ingroup utility_widgets
        * @defgroup utility_widgets-context Context Manager Widget
        * Contains classes related to the ContextManagerWidget.
        */

        /**
        * @ingroup utility_widgets
        * @defgroup utility_widgets-module Module Widget
        * Contains classes related to the ModuleWidget .
        */

        /**
        * @ingroup utility_widgets
        * @defgroup utility_widgets-grouping Grouping Widget
        * Contains classes related to the GroupingsWidget.
        */

        /**
        * @ingroup utility_widgets
        * @defgroup utility_widgets-selection_details Selection Details Widget
        * Contains classes related to the SelectionDetailsWidget.
        */

    /**
     * @ingroup gui
     * @defgroup unused Unused
     * Classes and systems that are no longer used or that are not (yet) available in a current master build.
     */


