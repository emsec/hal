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

namespace hal {
    class JsonWriteComplex;
    class JsonWriteObject;
    class JsonWriteArray;
    class JsonWriteDocument;

    class JsonWriteData
    {
        friend class JsonWriteComplex;
    protected:
        std::string mTagname;
        JsonWriteComplex* mParent;

        virtual rapidjson::Document::AllocatorType& allocator();
    public:
        JsonWriteData(const std::string& tag, JsonWriteComplex* parent);
        virtual ~JsonWriteData() {;}

        JsonWriteData& operator=(const std::string& txt);
        JsonWriteData& operator=(int ivalue);
    };

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
        JsonWriteComplex(const std::string& tag, JsonWriteComplex* parent);
        virtual ~JsonWriteComplex();
        void close();
    };

    class JsonWriteObject : public JsonWriteComplex
    {
        friend class JsonWriteData;
        friend class JsonWriteDocument;
    protected:

        virtual void add_member(rapidjson::Document::ValueType&& name,
                                rapidjson::Document::ValueType&& value,
                                rapidjson::Document::AllocatorType& allocator);
        virtual void finalize(JsonWriteComplex* cplx) override;
    public:
        JsonWriteObject(const std::string& tag, JsonWriteComplex* parent);
        virtual ~JsonWriteObject() {;}
        JsonWriteData& operator[](const std::string& tag);
        JsonWriteObject& add_object(const std::string& tag);
        JsonWriteArray& add_array(const std::string& tag);
    };

    class JsonWriteArray : public JsonWriteComplex
    {
        friend class JsonWriteData;
        friend class JsonWriteDocument;
    protected:
        virtual void finalize(JsonWriteComplex* cplx) override;
    public:
        JsonWriteArray(const std::string& tag, JsonWriteComplex* parent);
        virtual ~JsonWriteArray() {;}
        JsonWriteArray& operator <<(const std::string& txt);
        JsonWriteArray& operator <<(int ivalue);
        JsonWriteArray& add_array();
        JsonWriteObject& add_object();
    };

    class JsonWriteDocument : public JsonWriteObject
    {
        friend class JsonWriteData;
    protected:
        rapidjson::Document mRapidDocument;

        virtual rapidjson::Document::AllocatorType& allocator() override;
        virtual void finalize(JsonWriteComplex* cplx) override;
        virtual void add_member(rapidjson::Document::ValueType&& name,
                                rapidjson::Document::ValueType&& value,
                                rapidjson::Document::AllocatorType& allocator) override;
        void dump(rapidjson::Value& parent);
    public:
        JsonWriteDocument();
        virtual ~JsonWriteDocument() {;}
        bool serialize(const std::string& filename);
        void dump();
   };
}
