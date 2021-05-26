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

#include <QString>
namespace hal
{
    /**
     * @ingroup graph-contexts
     * @brief Interface for any class that wants to subscribe a GraphContext.
     *
     * Interface for a class that is a subscriber of a GraphContext. A subscriber will be notified about changes of the
     * layout status and about the deletion of the subscribed context.
     */
    class GraphContextSubscriber
    {
    public:
        virtual ~GraphContextSubscriber() = default;

        /**
         * Called whenever the scene becomes available again (i.e. after the GraphLayouter has finished its job).
         */
        virtual void handleSceneAvailable() = 0;

        /**
         * Called whenever the scene becomes unavailable (i.e. when the scene is blocked during the layouting process).
         */
        virtual void handleSceneUnavailable() = 0;

        /**
         * Called whenever the context is about to be deleted.
         */
        virtual void handleContextAboutToBeDeleted() = 0;

        /**
         * Called whenever the loading progress of the layouter of the subscribed context has changed.
         *
         * @param percent - The progress in percent (in range 0-100(?))
         */
        virtual void handleStatusUpdate(const int percent) = 0;

        /**
         * Called whenever the loading progress message of the layouter of the subscribed context has changed.
         *
         * @param message - The new message
         */
        virtual void handleStatusUpdate(const QString& message) = 0;
    };
}
