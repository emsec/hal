#pragma once
#include <map>
#include <unordered_map>
#include <functional>
#include <string>
#ifdef STANDALONE_PARSER
#include "saleae_directory.h"
#include "saleae_file.h"
#else
#include "netlist_simulator_controller/saleae_directory.h"
#include "netlist_simulator_controller/saleae_file.h"
#endif

namespace hal
{
    class SaleaeInputFile;

#ifdef STANDALONE_PARSER
    class Net
    {
      uint32_t mId;
      std::string mName;
    public:
      Net(const std::string n, uint32_t i=0) : mId(i), mName(n) {;}
      uint32_t get_id() const { return mId; }
      std::string get_name() const { return mName; }
    };
#else
    class Net;
#endif
  
    /**
     * @brief The SaleaeParser class is an engine which allows to parse any number of SALEAE files into a sequence of events ordered by time.
     * The SaleaeParser is used in the following way:
     * <ul>
     *   <li> Create an instance of parser with filename of SALEAE directory as mandatory argument. </li>
     *   <li> Register callback for each waveform events are requested. The callback function must have
     *        three mandatory arguments :<br>
     *        1) A parser-dont-care object that helps the callback function to identify the waveform.<br>
     *        2) Event time.<br>
     *        3) Waveform value. </li>
     *   <li> Repeat: call next_event() until function returns false. </li>
     *  </ul>
     */
    class SaleaeParser
    {
        SaleaeDirectory mSaleaeDirectory;
        static std::string strim(std::string s);
        struct DataFileHandle {
            std::function<void(void*obj, uint64_t, int)> callback;
            SaleaeInputFile* file;
            int value;
            void* obj;
        };
        std::multimap<uint64_t,DataFileHandle> mNextValueMap;

    public:
        /**
         * Constructor for SaleaeParser
         * @param filename Full path and filename of SALEAE directory file
         */
        SaleaeParser(const std::string& filename);

        /// Get last simulated time from SALEAE directory
        uint64_t get_max_time() const;

        /**
         * @brief Registers callback which gets executed upon next_event()
         * @param name Net/Waveform name for which callback gets registered
         * @param id Net/Waveform ID for which callback gets registered
         * @param callback Callback function with three mandatory arguments
         * @param obj Pointer to any object (e.g. net) that might be useful for callback function to identify net or waveform
         * @return True if callback was registered successfully, false otherwise (e.g. if net was not found in SALEAE directory)
         */
        bool register_callback(const std::string& name, uint32_t id, std::function<void(void*,uint64_t, int)> callback, void* obj);

        /**
         * @brief Registers callback which gets executed upon next_event()
         * @param net Net pointer corresponding to waveform for which callback gets registered
         * @param callback Callback function with three mandatory arguments
         * @param obj Pointer to any object (e.g. net) that might be useful for callback function to identify net or waveform
         * @return True if callback was registered successfully, false otherwise (e.g. if net was not found in SALEAE directory)
         */
        bool register_callback(const Net* net, std::function<void(void*,uint64_t, int)> callback, void* obj);

        /// Call to parser to deliver next event via registered callbacks (see above).
        bool next_event();

        /// Getter for SALEAE directory instance
        SaleaeDirectory get_directory() const { return mSaleaeDirectory; }

        /// conversion factor to be applied when converting double values from original SALEAE file to integer values
        static uint64_t sTimeScaleFactor;
    };
}
