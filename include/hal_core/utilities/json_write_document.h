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

#define RAPIDJSON_HAS_STDSTRING 1

#include "hal_core/defines.h"
#include "rapidjson/document.h"

#include <string>
#include <vector>

namespace hal
{
    class JsonWriteComplex;
    class JsonWriteObject;
    class JsonWriteArray;
    class JsonWriteDocument;

    /**
     * The JsonWriteData class represents a simple data field which gets
     * assigned using the operator=
     */
    class JsonWriteData
    {
        friend class JsonWriteComplex;

    protected:
        std::string mTagname;
        JsonWriteComplex* mParent;

        virtual rapidjson::Document::AllocatorType& allocator();

    public:
        /**
         * Constructor.
         * 
         * @param[in] tag - JSON tag name.
         * @param[in] parent - Parent instance.
         */
        JsonWriteData(const std::string& tag, JsonWriteComplex* parent);
        virtual ~JsonWriteData()
        {
            ;
        }

        /**
         * Assign text string to JSON field.
         * 
         * @param[in] txt - The text content.
         * @return The instance of the class.
         */
        JsonWriteData& operator=(const std::string& txt);

        /**
         * Assign int value to JSON field.
         * 
         * @param[in] ivalue - The numerical content (integer).
         * @return The instance of the class.
         */
        JsonWriteData& operator=(int ivalue);

        /**
         * Assign int value to JSON field.
         *
         * @param[in] value - The numerical content (floating point).
         * @return The instance of the class.
         */
        JsonWriteData& operator=(double value);

        /**
         * Assign unsigned 64 bit value to JSON field.
         * 
         * @param[in] u64Value - The numerical content.
         * @return The instance of the class.
         */
        JsonWriteData& operator=(uint64_t u64Value);
    };

    /**
     * The JsonWriteComplex class is the abstract base class for JSON objects and
     * arrays. Both complex types have in common that a call to close is
     * neccessary at the end to indicate that no more content will be added
     * to the element.
     */
    class JsonWriteComplex : public JsonWriteData
    {
        friend class JsonWriteData;
        friend class JsonWriteObject;
        friend class JsonWriteArray;
        friend class JsonWriteDocument;

    protected:
        std::vector<JsonWriteData*> mChildData;
        rapidjson::Value mRapidValue;

        virtual void finalize(JsonWriteComplex* cplx) = 0;

    public:
        /**
         * Constructor.
         * 
         * @param[in] tag - JSON tag name.
         * @param[in] parent - Parent instance.
         */
        JsonWriteComplex(const std::string& tag, JsonWriteComplex* parent);
        virtual ~JsonWriteComplex();

        /**
         * Must be called when no more data gets added to complex JSON element.
         */
        void close();
    };

    class JsonWriteObject : public JsonWriteComplex
    {
        friend class JsonWriteData;
        friend class JsonWriteDocument;

    protected:
        virtual void add_member(rapidjson::Document::ValueType&& name, rapidjson::Document::ValueType&& value, rapidjson::Document::AllocatorType& allocator);
        virtual void finalize(JsonWriteComplex* cplx) override;

    public:
        /**
         * Constructor.
         * 
         * @param[in] tag - JSON tag name.
         * @param[in] parent - Parent instance.
         */
        JsonWriteObject(const std::string& tag, JsonWriteComplex* parent);
        virtual ~JsonWriteObject()
        {
            ;
        }

        /**
         * Bracket operator to assign named tag.
         * 
         * @param[in] tag - The tag name.
         * @return The assigned JSON data instance.
         */
        JsonWriteData& operator[](const std::string& tag);

        /**
         * Add complex child object.
         * 
         * @param[in] tag - The tag name for child object.
         * @return The child object created by method.
         */
        JsonWriteObject& add_object(const std::string& tag);

        /**
         * Add child array.
         * 
         * @param[in] tag - The tag name for child array.
         * @return The child array created by method.
         */
        JsonWriteArray& add_array(const std::string& tag);
    };

    class JsonWriteArray : public JsonWriteComplex
    {
        friend class JsonWriteData;
        friend class JsonWriteDocument;

    protected:
        virtual void finalize(JsonWriteComplex* cplx) override;

    public:
        /**
         * Constructor.
         * 
         * @param[in] tag - JSON tag name.
         * @param[in] parent - Parent instance.
         */
        JsonWriteArray(const std::string& tag, JsonWriteComplex* parent);
        virtual ~JsonWriteArray()
        {
            ;
        }

        /**
         * Operator to add unnamed text field to array.
         * 
         * @param[in] txt - The text content.
         * @return The JSON array where the text was added.
         */
        JsonWriteArray& operator<<(const std::string& txt);

        /**
         * Operator to add an integer value to array.
         * 
         * @param[in] ivalue - The integer value.
         * @return The JSON array where the integer value was added.
         */
        JsonWriteArray& operator<<(int ivalue);

        /**
         * Add child array to array.
         * 
         * @return The JSON array where the array was added.
         */
        JsonWriteArray& add_array();

        /**
         * Add child array to array.
         * 
         * @return The JSON array where the array was added.
         */
        JsonWriteObject& add_object();
    };

    class JsonWriteDocument : public JsonWriteObject
    {
        friend class JsonWriteData;

    protected:
        rapidjson::Document mRapidDocument;

        virtual rapidjson::Document::AllocatorType& allocator() override;
        virtual void finalize(JsonWriteComplex* cplx) override;
        virtual void add_member(rapidjson::Document::ValueType&& name, rapidjson::Document::ValueType&& value, rapidjson::Document::AllocatorType& allocator) override;
        void dump(rapidjson::Value& parent);

    public:
        JsonWriteDocument();
        virtual ~JsonWriteDocument()
        {
            ;
        }

        /**
         * Serialize to file method.
         * 
         * @param[in] filename - The output filename.
         * @return `true` on success, `false` otherwise.
         */
        bool serialize(const std::string& filename);

        /**
         * Dump content for debugging purpose.
         */
        void dump();
    };
}    // namespace hal
