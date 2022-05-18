#include "hal_core/utilities/json_write_document.h"

#include "rapidjson/filereadstream.h"
#include "rapidjson/stringbuffer.h"

#include <fstream>

#define PRETTY_JSON_OUTPUT 0
#if PRETTY_JSON_OUTPUT == 1
#include "rapidjson/prettywriter.h"
#else
#include "rapidjson/writer.h"
#endif

namespace hal
{
    //--- Data -------
    JsonWriteData::JsonWriteData(const std::string& tag, JsonWriteComplex* parent) : mTagname(tag), mParent(parent)
    {
        ;
    }

    rapidjson::Document::AllocatorType& JsonWriteData::allocator()
    {
        JsonWriteComplex* p = mParent;
        if (!p)
            p = static_cast<JsonWriteObject*>(this);    // no parent: must be document
        else
            while (p->mParent)
            {
                p = p->mParent;
                assert(p != p->mParent);
            }
        JsonWriteDocument* doc = dynamic_cast<JsonWriteDocument*>(p);
        assert(doc);
        return doc->allocator();
    }

    JsonWriteData& JsonWriteData::operator=(const std::string& txt)
    {
        JsonWriteObject* p = dynamic_cast<JsonWriteObject*>(mParent);
        assert(p);
        p->add_member(rapidjson::Value(mTagname, allocator()), rapidjson::Value(txt, allocator()), allocator());
        return *this;
    }

    JsonWriteData& JsonWriteData::operator=(int ivalue)
    {
        JsonWriteObject* p = dynamic_cast<JsonWriteObject*>(mParent);
        assert(p);
        p->add_member(rapidjson::Value(mTagname, allocator()), rapidjson::Value(ivalue), allocator());
        return *this;
    }

    JsonWriteData& JsonWriteData::operator=(uint64_t u64Value)
    {
        JsonWriteObject* p = dynamic_cast<JsonWriteObject*>(mParent);
        assert(p);
        p->add_member(rapidjson::Value(mTagname, allocator()), rapidjson::Value(u64Value), allocator());
        return *this;
    }

    JsonWriteData& JsonWriteData::operator=(double value)
    {
        JsonWriteObject* p = dynamic_cast<JsonWriteObject*>(mParent);
        assert(p);
        p->add_member(rapidjson::Value(mTagname, allocator()), rapidjson::Value(value), allocator());
        return *this;
    }

    //--- Complex ----
    JsonWriteComplex::JsonWriteComplex(const std::string& tag, JsonWriteComplex* parent) : JsonWriteData(tag, parent)
    {
        ;
    }

    JsonWriteComplex::~JsonWriteComplex()
    {
        for (JsonWriteData* obj : mChildData)
            delete obj;
        mChildData.clear();
    }

    void JsonWriteComplex::close()
    {
        mParent->finalize(this);
    }

    //--- Object -----
    JsonWriteObject::JsonWriteObject(const std::string& tag, JsonWriteComplex* parent) : JsonWriteComplex(tag, parent)
    {
        mRapidValue.SetObject();
    }

    void JsonWriteObject::add_member(rapidjson::Document::ValueType&& name, rapidjson::Document::ValueType&& value, rapidjson::Document::AllocatorType& allocator)
    {
        mRapidValue.AddMember(name, value, allocator);
    }

    JsonWriteData& JsonWriteObject::operator[](const std::string& tag)
    {
        JsonWriteData* dat = new JsonWriteData(tag, this);
        mChildData.push_back(dat);
        return *dat;
    }

    JsonWriteObject& JsonWriteObject::add_object(const std::string& tag)
    {
        JsonWriteObject* obj = new JsonWriteObject(tag, this);
        mChildData.push_back(obj);
        return *obj;
    }

    JsonWriteArray& JsonWriteObject::add_array(const std::string& tag)
    {
        JsonWriteArray* arr = new JsonWriteArray(tag, this);
        mChildData.push_back(arr);
        return *arr;
    }

    void JsonWriteObject::finalize(JsonWriteComplex* cplx)
    {
        mRapidValue.AddMember(rapidjson::Value(cplx->mTagname, allocator()), cplx->mRapidValue, allocator());
    }

    //--- Array ------
    JsonWriteArray::JsonWriteArray(const std::string& tag, JsonWriteComplex* parent) : JsonWriteComplex(tag, parent)
    {
        mRapidValue.SetArray();
    }

    JsonWriteArray& JsonWriteArray::operator<<(const std::string& txt)
    {
        mRapidValue.PushBack(rapidjson::Value(txt, allocator()), allocator());
        return *this;
    }

    JsonWriteArray& JsonWriteArray::operator<<(int ivalue)
    {
        mRapidValue.PushBack(rapidjson::Value(ivalue), allocator());
        return *this;
    }

    JsonWriteArray& JsonWriteArray::add_array()
    {
        JsonWriteArray* arr = new JsonWriteArray(std::string(), this);
        mChildData.push_back(arr);
        return *arr;
    }

    JsonWriteObject& JsonWriteArray::add_object()
    {
        JsonWriteObject* obj = new JsonWriteObject(std::string(), this);
        mChildData.push_back(obj);
        return *obj;
    }

    void JsonWriteArray::finalize(JsonWriteComplex* cplx)
    {
        mRapidValue.PushBack(cplx->mRapidValue, allocator());
    }

    //--- Document ---
    JsonWriteDocument::JsonWriteDocument() : JsonWriteObject(std::string(), nullptr)
    {
        mRapidDocument.SetObject();
    }

    rapidjson::Document::AllocatorType& JsonWriteDocument::allocator()
    {
        return mRapidDocument.GetAllocator();
    }

    void JsonWriteDocument::finalize(JsonWriteComplex* cplx)
    {
        mRapidDocument.AddMember(rapidjson::Value(cplx->mTagname, allocator()), cplx->mRapidValue, allocator());
    }

    void JsonWriteDocument::add_member(rapidjson::Document::ValueType&& name, rapidjson::Document::ValueType&& value, rapidjson::Document::AllocatorType& allocator)
    {
        mRapidDocument.AddMember(name, value, allocator);
    }

    bool JsonWriteDocument::serialize(const std::string& filename)
    {
        std::ofstream of(filename);
        if (!of.good())
            return false;

        rapidjson::StringBuffer strbuf;
#if PRETTY_JSON_OUTPUT == 1
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
#else
        rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
#endif
        mRapidDocument.Accept(writer);
        of << strbuf.GetString();
        of.close();
        return true;
    }

    void JsonWriteDocument::dump()
    {
        dump(mRapidDocument);
    }

    void JsonWriteDocument::dump(rapidjson::Value& parent)
    {
        UNUSED(parent);
        /*        
        for (rapidjson::Document::MemberIterator it = parent.MemberBegin(); it!=parent.MemberEnd(); ++it)
        {
            qDebug() << "x" << it->name.GetString();
            switch(it->value.GetType())
            {
            case rapidjson::kNumberType:
                qDebug() << it->value.GetInt();
                break;
            case rapidjson::kStringType:
                qDebug() << it->value.GetString();
                break;
            case rapidjson::kObjectType:
                if (parent.HasMember("name"))
                    qDebug() << "***name***" << parent["name"].GetString();
                dump(it->value);
                break;
            case rapidjson::kArrayType:
                for (rapidjson::Document::ConstValueIterator jt = it->value.Begin(); jt!=it->value.End(); ++jt)
                    qDebug() << jt->GetString();
                break;
            default:
                qDebug() << "type not handled" << it->value.GetType();
                break;
            }
        }
    */
    }
}    // namespace hal
