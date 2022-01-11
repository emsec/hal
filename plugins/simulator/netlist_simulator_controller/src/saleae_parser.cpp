#include "netlist_simulator_controller/saleae_parser.h"
#include "netlist_simulator_controller/saleae_file.h"
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <cctype>
#include <filesystem>
#include <iostream>

namespace hal
{
    uint64_t SaleaeParser::sTimeScaleFactor = 1000000000;

        SaleaeParser::SaleaeParser(const std::string& filename)
        : mCsvFilename(filename)
    {
        std::ifstream ff(mCsvFilename);
        std::string line;
        bool isKeyword = true;
        while (std::getline(ff, line))
        {
            if (isKeyword)
            {
                if (line != "<SALEAE>") return;
                isKeyword = false;
            }

            std::istringstream iss(line);
            std::string token;
            bool isName = false;

            std::string abbrev;
            std::string name;
            while (std::getline(iss, token, ','))
            {
                if (isName)
                    name = strim(token);
                else
                    abbrev = strim(token);
                isName = true;
            }
            mSaleaeAbbrevMap[name] = abbrev;
        }
    }

    std::string SaleaeParser::strim(std::string s)
    {
        if (s.empty()) return s;
        int p0 = 0;
        int p1 = s.length()-1;
        while (p0 <= p1 && isspace(s.at(p0))) p0++;
        while (p0 <= p1 && isspace(s.at(p1))) p1--;
        if (s.at(p0) == '"' && s.at(p1) == '"' && p1-p0 >= 2)
        {
            ++p0;
            --p1;
        }
        return s.substr(p0,p1-p0+1);
    }

    bool SaleaeParser::nextEvent()
    {
        auto it = mNextValueMap.begin();
        if (it == mNextValueMap.end()) return false;
        WaveFormFile wff = it->second;
        mNextValueMap.erase(it);
        wff.callback(wff.obj,it->first,wff.value);
        wff.value = wff.value ? 0 : 1;
        if (wff.file->good())
        {
            uint64_t nextT = wff.file->nextTimeValue();
            mNextValueMap.insert(std::pair<uint64_t,WaveFormFile>(nextT,wff));
        }
        else
            delete wff.file;
        return true;
    }

    u64 SaleaeParser::get_max_time() const
    {
        u64 retval = 0;
        for (auto it = mSaleaeAbbrevMap.begin(); it != mSaleaeAbbrevMap.end(); ++it)
        {
            SaleaeInputFile* sif = inputFileFactory(it->second);
            if (sif->good())
            {
                if (sif->header()->endTime() > retval)
                    retval = sif->header()->endTime();
            }
            delete sif;
        }
        return retval;
    }

    SaleaeInputFile* SaleaeParser::inputFileFactory(const std::string& abbrev) const
    {
        std::filesystem::path path(mCsvFilename);
        path.replace_filename(std::string("digital_") + abbrev + ".bin");
        return new SaleaeInputFile(path.string());
    }

    bool SaleaeParser::registerCallback(std::string& name, std::function<void(const void*,uint64_t,int)> callback, const void *obj)
    {
        auto it = mSaleaeAbbrevMap.find(name);
        if (it == mSaleaeAbbrevMap.end()) return false;
        std::filesystem::path path(mCsvFilename);
        SaleaeInputFile* datafile = inputFileFactory(it->second);
        if (!datafile->good())
        {
            std::cerr << "Error loading SALEAE datafile <" << datafile->get_last_error() << ">" << std::endl;
            delete datafile;
            return false;
        }
        mNextValueMap.insert(std::pair<uint64_t,WaveFormFile>(0,{callback, name, datafile, datafile->startValue(), obj}));
        return true;
    }
}
