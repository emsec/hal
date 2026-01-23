#include "hawkeye/sbox_database.h"

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

// debug options to enforce using default implementation
// #undef __AVX2__
// #undef __ARM_NEON

#ifdef __AVX2__
#include <immintrin.h>

using smallset_t = __m256i;
#elif defined(__ARM_NEON)
#include <arm_neon.h>

using smallset_t = uint64x2x2_t;
#else
const uint64_t _ONE_ = 1;

class smallset_t
{
public:
    smallset_t(int preset = 0);

    void set(u8 bit);
    bool is_set(u8 bit) const;

    void dump() const;

    smallset_t operator|(const smallset_t& other) const;
    smallset_t operator&(const smallset_t& other) const;
    smallset_t operator^(const smallset_t& other) const;
    smallset_t shuffle(u8 shift) const;

    void to_array(u64* arr, bool swap = false) const;

    u8 least_bit() const;
    static int least_bit(u64 dw);

    int size() const;
    static int size(u64 dw, int level);

    bool empty() const;

private:
    u64 dw64[4];
};

smallset_t::smallset_t(int preset)
{
    memset(dw64, 0, sizeof(dw64));
    switch (preset)
    {
        case 8:
            dw64[0] = 0xFF;
            break;
        case 16:
            dw64[0] = 0xFFFF;
            break;
        case 32:
            dw64[0] = 0xFFFFFFFF;
            break;
        case 64:
            memset(dw64, 0xFF, sizeof(u64));
            break;
        case 128:
            memset(dw64, 0xFF, 2 * sizeof(u64));
            break;
        case 256:
            memset(dw64, 0xFF, sizeof(dw64));
            break;
    }
}

bool smallset_t::empty() const
{
    for (int i = 0; i < 4; i++)
    {
        if (dw64[i])
        {
            return false;
        }
    }
    return true;
}

u8 smallset_t::least_bit() const
{
    for (int i = 0; i < 4; i++)
    {
        if (dw64[i])
        {
            return i * 64 + least_bit(dw64[i]);
        }
    }
    std::cerr << "Called smallset_t::least_bit() on empty set\n" << std::endl;
    return 0;
}

smallset_t smallset_t::shuffle(u8 shift) const
{
    smallset_t retval(*this);
    if (shift & 0x80)
    {
        smallset_t temp = retval;
        retval.dw64[0]  = temp.dw64[2];
        retval.dw64[1]  = temp.dw64[3];
        retval.dw64[2]  = temp.dw64[0];
        retval.dw64[3]  = temp.dw64[1];
    }
    if (shift & 0x40)
    {
        smallset_t temp = retval;
        retval.dw64[0]  = temp.dw64[1];
        retval.dw64[1]  = temp.dw64[0];
        retval.dw64[2]  = temp.dw64[3];
        retval.dw64[3]  = temp.dw64[2];
    }
    if (shift & 0x20)
    {
        for (int i = 0; i < 4; i++)
        {
            retval.dw64[i] = ((retval.dw64[i] & 0xFFFFFFFF00000000ULL) >> 32) | ((retval.dw64[i] & 0x00000000FFFFFFFFULL) << 32);
        }
    }
    if (shift & 0x10)
    {
        for (int i = 0; i < 4; i++)
        {
            retval.dw64[i] = ((retval.dw64[i] & 0xFFFF0000FFFF0000ULL) >> 16) | ((retval.dw64[i] & 0x0000FFFF0000FFFFULL) << 16);
        }
    }
    if (shift & 0x08)
    {
        for (int i = 0; i < 4; i++)
        {
            retval.dw64[i] = ((retval.dw64[i] & 0xFF00FF00FF00FF00ULL) >> 8) | ((retval.dw64[i] & 0x00FF00FF00FF00FFULL) << 8);
        }
    }
    if (shift & 0x04)
    {
        for (int i = 0; i < 4; i++)
        {
            retval.dw64[i] = ((retval.dw64[i] & 0xF0F0F0F0F0F0F0F0ULL) >> 4) | ((retval.dw64[i] & 0x0F0F0F0F0F0F0F0FULL) << 4);
        }
    }
    if (shift & 0x02)
    {
        for (int i = 0; i < 4; i++)
        {
            retval.dw64[i] = ((retval.dw64[i] & 0xCCCCCCCCCCCCCCCCULL) >> 2) | ((retval.dw64[i] & 0x3333333333333333ULL) << 2);
        }
    }
    if (shift & 0x01)
    {
        for (int i = 0; i < 4; i++)
        {
            retval.dw64[i] = ((retval.dw64[i] & 0xAAAAAAAAAAAAAAAAULL) >> 1) | ((retval.dw64[i] & 0x5555555555555555ULL) << 1);
        }
    }
    return retval;
}

void smallset_t::to_array(u64* arr, bool swap) const
{
    for (int i = 0; i < 4; i++)
    {
        arr[i] = dw64[swap ? 3 - i : i];
    }
}

void smallset_t::set(u8 bit)
{
    dw64[bit / 64] |= (_ONE_ << (bit % 64));
}

bool smallset_t::is_set(u8 bit) const
{
    return (dw64[bit / 64] & (_ONE_ << (bit % 64))) != 0;
}

int smallset_t::size() const
{
    int retval = 0;
    for (int i = 0; i < 4; i++)
        retval += size(dw64[i], 0);
    return retval;
}

int smallset_t::size(u64 dw, int level)
{
    static const u64 segmask[]    = {0xFFFFFFFF, 0xFFFF, 0xFF, 0xF};
    static const int segshft[]    = {32, 16, 8, 4};
    static const int szlookup[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

    if (level >= 4)
        return szlookup[dw & 0xF];

    int retval = 0;
    retval += size(dw & segmask[level], level + 1);
    dw >>= segshft[level];
    retval += size(dw & segmask[level], level + 1);

    return retval;
}

int smallset_t::least_bit(u64 dw)
{
    static const u64 segmask[]    = {0xFFFFFFFF, 0xFFFF, 0xFF, 0xF};
    static const int lblookup[16] = {-61, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};

    int retval = 0;
    int segval = 32;

    for (int iseg = 0; iseg < 4; iseg++)
    {
        if (!(dw & segmask[iseg]))
        {
            retval += segval;
            dw >>= segval;
        }
        segval /= 2;
    }

    return retval + lblookup[dw & 0xF];
}

void smallset_t::dump() const
{
    for (int i = 3; i >= 0; i--)
        printf("%016lx", dw64[i]);
    printf("\n");

    for (unsigned int i = 0; i < 256; i++)
    {
        if (dw64[i / 64] & (_ONE_ << (i % 64)))
            printf("%8d\n", i);
    }
}

smallset_t smallset_t::operator|(const smallset_t& other) const
{
    smallset_t retval = other;
    for (int i = 0; i < 4; i++)
        retval.dw64[i] |= dw64[i];
    return retval;
}

smallset_t smallset_t::operator&(const smallset_t& other) const
{
    smallset_t retval = other;
    for (int i = 0; i < 4; i++)
        retval.dw64[i] &= dw64[i];
    return retval;
}

smallset_t smallset_t::operator^(const smallset_t& other) const
{
    smallset_t retval = other;
    for (int i = 0; i < 4; i++)
        retval.dw64[i] ^= dw64[i];
    return retval;
}

#endif

namespace hal
{
    namespace hawkeye
    {
        SBoxDatabase::SBoxDatabase(const std::map<std::string, std::vector<u8>>& sboxes)
        {
            add(sboxes).is_ok();
        }

        Result<SBoxDatabase> SBoxDatabase::from_file(const std::filesystem::path& file_path)
        {
            auto db = SBoxDatabase();
            if (const auto res = db.load(file_path); res.is_ok())
            {
                return OK(db);
            }
            else
            {
                return ERR(res.get_error());
            }
        }

        Result<std::monostate> SBoxDatabase::add(const std::string& name, const std::vector<u8>& sbox)

        {
            u32 bit_size = std::log2(sbox.size());

            if (bit_size > 8)
            {
                return ERR("S-box '" + name + "' has bit-size greater 8, but only S-boxes of up to 8 bits are supported");
            }

            for (size_t alpha = 0; alpha < sbox.size(); alpha++)
            {
                std::vector<u8> sbox_alpha;
                for (u32 i = 0; i < sbox.size(); i++)
                {
                    sbox_alpha.push_back(sbox.at(i) ^ alpha);
                }
                auto lin_rep = compute_linear_representative(sbox_alpha);
                m_data[bit_size][lin_rep].push_back(std::make_pair(name, alpha));
            }
            return OK({});
        }

        Result<std::monostate> SBoxDatabase::add(const std::map<std::string, std::vector<u8>>& sboxes)
        {
            for (const auto& [name, sbox] : sboxes)
            {
                if (const auto res = add(name, sbox); res.is_error())
                {
                    return ERR(res.get_error());
                }
            }
            return OK({});
        }

        Result<std::monostate> SBoxDatabase::load(const std::filesystem::path& file_path, bool overwrite)
        {
            FILE* fp = fopen(file_path.string().c_str(), "r");
            if (fp == NULL)
            {
                return ERR("could not parse S-box database file '" + file_path.string() + "' : unable to open file");
            }

            char buffer[65536];
            rapidjson::FileReadStream is(fp, buffer, sizeof(buffer));
            rapidjson::Document document;
            document.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(is);
            fclose(fp);

            if (document.HasParseError())
            {
                return ERR("could not parse S-box database file '" + file_path.string() + "': failed parsing JSON format");
            }

            if (overwrite)
            {
                m_data.clear();
            }

            for (auto size_it = document.MemberBegin(); size_it != document.MemberEnd(); ++size_it)
            {
                u32 bit_size                       = std::stoul(std::string(size_it->name.GetString()));
                const rapidjson::Value& cipher_val = size_it->value;

                for (auto cipher_it = cipher_val.MemberBegin(); cipher_it != cipher_val.MemberEnd(); ++cipher_it)
                {
                    std::string cipher_name           = cipher_it->name.GetString();
                    const rapidjson::Value& const_val = cipher_it->value;

                    for (auto const_it = const_val.MemberBegin(); const_it != const_val.MemberEnd(); ++const_it)
                    {
                        u8 const_alpha                      = (u8)std::stoul(std::string(const_it->name.GetString()));
                        const rapidjson::Value& lin_rep_val = const_it->value;

                        std::vector<u8> lin_rep;
                        for (u32 i = 0; i < lin_rep_val.Size(); i++)
                        {
                            lin_rep.push_back((u8)(lin_rep_val[i].GetUint()));
                        }

                        m_data[bit_size][lin_rep].push_back(std::make_pair(cipher_name, const_alpha));
                    }
                }
            }

            return OK({});
        }

        Result<std::monostate> SBoxDatabase::store(const std::filesystem::path& file_path) const
        {
            FILE* fp = fopen(file_path.string().c_str(), "w");
            if (fp == NULL)
            {
                return ERR("could not write S-box database file '" + file_path.string() + "' : unable to open file");
            }

            rapidjson::Document document;
            document.SetObject();

            rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

            for (const auto& [bit_size, lin_rep_map] : m_data)
            {
                std::map<std::string, std::map<u8, std::vector<u8>>> pretty_data;
                for (const auto& [lin_rep, cipher_vec] : lin_rep_map)
                {
                    for (const auto& [name, alpha] : cipher_vec)
                    {
                        pretty_data[name][alpha] = lin_rep;
                    }
                }

                rapidjson::Value cipher_json(rapidjson::kObjectType);
                for (const auto& [cipher_name, lin_rep_map] : pretty_data)
                {
                    rapidjson::Value alpha_json(rapidjson::kObjectType);
                    for (const auto& [const_alph, lin_rep] : lin_rep_map)
                    {
                        rapidjson::Value lin_rep_json(rapidjson::kArrayType);
                        for (const auto val : lin_rep)
                        {
                            lin_rep_json.PushBack(val, allocator);
                        }
                        alpha_json.AddMember(rapidjson::Value(std::to_string(const_alph).c_str(), allocator).Move(), lin_rep_json, allocator);
                    }
                    cipher_json.AddMember(rapidjson::Value(cipher_name.c_str(), allocator).Move(), alpha_json, allocator);
                }
                document.AddMember(rapidjson::Value(std::to_string(bit_size).c_str(), allocator).Move(), cipher_json, allocator);
            }

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

            document.Accept(writer);

            std::ofstream file(file_path);
            file << buffer.GetString();
            file.close();

            return ERR("not implemented");
        }

        Result<std::string> SBoxDatabase::lookup(const std::vector<u8>& sbox) const
        {
            u32 bit_size = std::log2(sbox.size());

            if (bit_size > 8)
            {
                return ERR("S-box has bit-size greater 8, but only S-boxes of up to 8 bits are supported");
            }

            const auto size_it = m_data.find(bit_size);
            if (size_it == m_data.end())
            {
                return ERR("no S-box of matching bit-size of " + std::to_string(bit_size) + " bits contained in database");
            }

            for (u8 beta = 0; beta < sbox.size(); beta++)
            {
                std::vector<u8> sbox_beta;
                for (u32 i = 0; i < sbox.size(); i++)
                {
                    sbox_beta.push_back(sbox.at(i) ^ beta);
                }
                auto lin_rep = compute_linear_representative(sbox_beta);

                const auto& matching_size_data = std::get<1>(*size_it);
                const auto rep_it              = matching_size_data.find(lin_rep);
                if (rep_it != matching_size_data.end())
                {
                    return OK(rep_it->second.front().first);
                }
            }

            return ERR("no match found within database");
        }

        void SBoxDatabase::print() const
        {
            for (const auto& [bit_size, lin_rep_map] : m_data)
            {
                std::cout << std::endl;
                std::cout << "### WIDTH: " << bit_size << std::endl;
                std::cout << "#######################" << std::endl;

                std::map<std::string, std::map<u8, std::vector<u8>>> pretty_data;
                for (const auto& [lin_rep, cipher_vec] : lin_rep_map)
                {
                    for (const auto& [name, alpha] : cipher_vec)
                    {
                        pretty_data[name][alpha] = lin_rep;
                    }
                }

                for (const auto& [cipher_name, lin_rep_map] : pretty_data)
                {
                    std::cout << "* " << cipher_name << std::endl;

                    for (const auto& [const_alph, lin_rep] : lin_rep_map)
                    {
                        std::cout << " - " << (u32)const_alph << ": [" << (u32)(lin_rep.at(0));
                        for (u32 i = 1; i < lin_rep.size(); i++)
                        {
                            std::cout << ", " << (u32)(lin_rep.at(i));
                        }
                        std::cout << "]" << std::endl;
                    }
                }
            }

            std::cout << std::endl;
        }

        namespace
        {
            void smallset_print(const std::string& name, const smallset_t& a)
            {
                u64 elements[4];
#ifdef __AVX2__

                elements[0] = _mm256_extract_epi64(a, 3);
                elements[1] = _mm256_extract_epi64(a, 2);
                elements[2] = _mm256_extract_epi64(a, 1);
                elements[3] = _mm256_extract_epi64(a, 0);
#elif defined(__ARM_NEON)
                elements[0] = a.val[1][1];
                elements[1] = a.val[1][0];
                elements[2] = a.val[0][1];
                elements[3] = a.val[0][0];
#else
                a.to_array(elements, true);
#endif
                std::cout << name << ": 0b";
                for (u32 i = 0; i < 4; i++)
                {
                    for (int j = 63; j >= 0; j--)
                    {
                        u32 bit = (elements[i] >> j) & 1;
                        std::cout << bit;
                    }
                    std::cout << " ";
                }
                std::cout << std::endl;
            }

            u8 smallset_least_element(const smallset_t& a)
            {
                u64 chunks[4];
#ifdef __AVX2__
                chunks[0] = _mm256_extract_epi64(a, 0);
                chunks[1] = _mm256_extract_epi64(a, 1);
                chunks[2] = _mm256_extract_epi64(a, 2);
                chunks[3] = _mm256_extract_epi64(a, 3);
#elif defined(__ARM_NEON)
                chunks[0] = a.val[0][0];
                chunks[1] = a.val[0][1];
                chunks[2] = a.val[1][0];
                chunks[3] = a.val[1][1];
#else
                return a.least_bit();
#endif
                for (u32 i = 0; i < 4; i++)
                {
                    u64 current_chunk = chunks[i];
                    if (current_chunk != 0)
                    {
                        u8 idx = __builtin_ctzll(current_chunk) + i * 64;
                        return idx;
                    }
                }

                // set is empty -- caller's fault
                std::cout << "CALLED LEAST ELEMENT ON EMPTY SET!" << std::endl;
                return 0;
            }

            inline smallset_t smallset_intersect(const smallset_t& a, const smallset_t& b)
            {
#ifdef __AVX2__
                return _mm256_and_si256(a, b);
#elif defined(__ARM_NEON)
                return {vandq_u64(a.val[0], b.val[0]), vandq_u64(a.val[1], b.val[1])};
#else
                return (a & b);
#endif
            }

            inline smallset_t smallset_union(const smallset_t& a, const smallset_t& b)
            {
#ifdef __AVX2__
                return _mm256_or_si256(a, b);
#elif defined(__ARM_NEON)
                return {vorrq_u64(a.val[0], b.val[0]), vorrq_u64(a.val[1], b.val[1])};
#else
                return (a | b);
#endif
            }

            inline u16 smallset_size(const smallset_t& a)
            {
                u16 count = 0;
#ifdef __AVX2__
                u64 chunk = _mm256_extract_epi64(a, 0);
                count += __builtin_popcountll(chunk);
                chunk = _mm256_extract_epi64(a, 1);
                count += __builtin_popcountll(chunk);
                chunk = _mm256_extract_epi64(a, 2);
                count += __builtin_popcountll(chunk);
                chunk = _mm256_extract_epi64(a, 3);
                count += __builtin_popcountll(chunk);
#elif defined(__ARM_NEON)
                count += __builtin_popcountll(a.val[0][0]);
                count += __builtin_popcountll(a.val[0][1]);
                count += __builtin_popcountll(a.val[1][0]);
                count += __builtin_popcountll(a.val[1][1]);
#else
                return a.size();
#endif
                return count;
            }

            inline bool smallset_is_empty(const smallset_t& a)
            {
#ifdef __AVX2__
                return _mm256_testz_si256(a, a);
#elif defined(__ARM_NEON)
                auto tmp = vandq_u64(vceqzq_u64(a.val[0]), vceqzq_u64(a.val[1]));
                return (tmp[0] & tmp[1]) & 1;
#else
                return a.empty();
#endif
            }

            smallset_t smallset_add_element(const smallset_t& a, const u8 elm)
            {
                // compute union of a and {elm}
                u32 index = elm / 64;
#ifdef __AVX2__
                u64 mask[4]   = {0};
                mask[index]   = (u64)1 << (elm % 64);
                __m256i _mask = _mm256_set_epi64x(mask[3], mask[2], mask[1], mask[0]);
                return _mm256_or_si256(a, _mask);
#elif defined(__ARM_NEON)
                u64 mask[2]     = {0};
                mask[index & 1] = (u64)1 << (elm % 64);
                auto _mask      = vld1q_u64(mask);
                if (index < 2)
                {
                    return {vorrq_u64(a.val[0], _mask), a.val[1]};
                }
                else
                {
                    return {a.val[0], vorrq_u64(a.val[1], _mask)};
                }
#else
                smallset_t retval(a);
                retval.set(elm);
                return retval;
#endif
            }

            smallset_t smallset_shift(const smallset_t& b, const u8 shift)
            {
#if !defined(__AVX2__) && !defined(__ARM_NEON)
                return b.shuffle(shift);
#endif

                auto a = b;
                // compute a \oplus shift
                if ((shift >> 7) & 0x1)
                {
#ifdef __AVX2__
                    a = _mm256_permute2x128_si256(a, a, 1);
#elif defined(__ARM_NEON)
                    a.val[0] = b.val[1];
                    a.val[1] = b.val[0];
#endif
                }
                if ((shift >> 6) & 0x1)
                {
#ifdef __AVX2__
                    a = _mm256_permute4x64_epi64(a, _MM_SHUFFLE(2, 3, 0, 1));
#elif defined(__ARM_NEON)
                    a.val[0] = vextq_u64(a.val[0], a.val[0], 1);
                    a.val[1] = vextq_u64(a.val[1], a.val[1], 1);
#endif
                }
                if ((shift >> 5) & 0x1)
                {
#ifdef __AVX2__
                    a = _mm256_shuffle_epi32(a, _MM_SHUFFLE(2, 3, 0, 1));
#elif defined(__ARM_NEON)
                    a.val[0] = (uint64x2_t) vrev64q_u32((uint32x4_t) a.val[0]);
                    a.val[1] = (uint64x2_t) vrev64q_u32((uint32x4_t) a.val[1]);

#endif
                }
                if ((shift >> 4) & 0x1)
                {
#ifdef __AVX2__
                    a = _mm256_shufflelo_epi16(a, _MM_SHUFFLE(2, 3, 0, 1));
                    a = _mm256_shufflehi_epi16(a, _MM_SHUFFLE(2, 3, 0, 1));
#elif defined(__ARM_NEON)
                    a.val[0] = (uint64x2_t) vrev64q_u16((uint16x8_t) a.val[0]);
                    a.val[0] = (uint64x2_t) vrev64q_u32((uint32x4_t) a.val[0]);
                    a.val[1] = (uint64x2_t) vrev64q_u16((uint16x8_t) a.val[1]);
                    a.val[1] = (uint64x2_t) vrev64q_u32((uint32x4_t) a.val[1]);
#endif
                }
                if ((shift >> 3) & 0x1)
                {
#ifdef __AVX2__
                    const __m256i mask = _mm256_set_epi8(14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1, 14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1);
                    a                  = _mm256_shuffle_epi8(a, mask);
#elif defined(__ARM_NEON)
                    a.val[0] = (uint64x2_t) vrev64q_u8 ((uint8x16_t) a.val[0]);
                    a.val[0] = (uint64x2_t) vrev64q_u16((uint16x8_t) a.val[0]);
                    a.val[1] = (uint64x2_t) vrev64q_u8 ((uint8x16_t) a.val[1]);
                    a.val[1] = (uint64x2_t) vrev64q_u16((uint16x8_t) a.val[1]);
#endif
                }
                if ((shift >> 2) & 0x1)
                {
#ifdef __AVX2__
                    const __m256i mask_high = _mm256_set1_epi8((char)0xF0);
                    const __m256i mask_low  = _mm256_set1_epi8(0x0F);
                    const __m256i high      = _mm256_and_si256(a, mask_high);
                    const __m256i low       = _mm256_and_si256(a, mask_low);
                    a                       = _mm256_or_si256(_mm256_srli_epi16(high, 4), _mm256_slli_epi16(low, 4));
#elif defined(__ARM_NEON)
                    const auto mask_high = vdupq_n_u64(0xF0F0F0F0F0F0F0F0);
                    const auto mask_low  = vdupq_n_u64(0x0F0F0F0F0F0F0F0F);

                    for (u32 i = 0; i < 2; i++)
                    {
                        const auto high = vandq_u64(a.val[i], mask_high);
                        const auto low  = vandq_u64(a.val[i], mask_low);

                        a.val[i] = vorrq_u64(vshrq_n_u64(high, 4), vshlq_n_u64(low, 4));
                    }
#endif
                }
                if ((shift >> 1) & 0x1)
                {
#ifdef __AVX2__
                    const __m256i mask_high = _mm256_set1_epi8((char)0xCC);
                    const __m256i mask_low  = _mm256_set1_epi8(0x33);
                    const __m256i high      = _mm256_and_si256(a, mask_high);
                    const __m256i low       = _mm256_and_si256(a, mask_low);
                    a                       = _mm256_or_si256(_mm256_srli_epi16(high, 2), _mm256_slli_epi16(low, 2));
#elif defined(__ARM_NEON)
                    const auto mask_high = vdupq_n_u64(0xCCCCCCCCCCCCCCCC);
                    const auto mask_low  = vdupq_n_u64(0x3333333333333333);

                    for (u32 i = 0; i < 2; i++)
                    {
                        const auto high = vandq_u64(a.val[i], mask_high);
                        const auto low  = vandq_u64(a.val[i], mask_low);

                        a.val[i] = vorrq_u64(vshrq_n_u64(high, 2), vshlq_n_u64(low, 2));
                    }
#endif
                }
                if (shift & 0x1)
                {
#ifdef __AVX2__
                    const __m256i mask_high = _mm256_set1_epi8((char)0xAA);
                    const __m256i mask_low  = _mm256_set1_epi8(0x55);
                    const __m256i high      = _mm256_and_si256(a, mask_high);
                    const __m256i low       = _mm256_and_si256(a, mask_low);
                    a                       = _mm256_or_si256(_mm256_srli_epi16(high, 1), _mm256_slli_epi16(low, 1));
#elif defined(__ARM_NEON)
                    const auto mask_high = vdupq_n_u64(0xAAAAAAAAAAAAAAAA);
                    const auto mask_low  = vdupq_n_u64(0x5555555555555555);

                    for (u32 i = 0; i < 2; i++)
                    {
                        const auto high = vandq_u64(a.val[i], mask_high);
                        const auto low  = vandq_u64(a.val[i], mask_low);

                        a.val[i] = vorrq_u64(vshrq_n_u64(high, 1), vshlq_n_u64(low, 1));
                    }
#endif
                }
                return a;
            }

            smallset_t smallset_shift_union(const smallset_t& a, const u8 shift)
            {
                smallset_t b = smallset_shift(a, shift);
                return smallset_union(a, b);
            }

            std::vector<u8> smallset_get_elements(const smallset_t& a)
            {
                std::vector<u8> e;
                u64 chunks[4];
#ifdef __AVX2__
                chunks[0] = _mm256_extract_epi64(a, 0);
                chunks[1] = _mm256_extract_epi64(a, 1);
                chunks[2] = _mm256_extract_epi64(a, 2);
                chunks[3] = _mm256_extract_epi64(a, 3);
#elif defined(__ARM_NEON)
                chunks[0] = a.val[0][0];
                chunks[1] = a.val[0][1];
                chunks[2] = a.val[1][0];
                chunks[3] = a.val[1][1];
#else
                a.to_array(chunks);
#endif
                for (u32 i = 0; i < 4; i++)
                {
                    u64 current_chunk = chunks[i];
                    while (current_chunk != 0)
                    {
                        u8 idx = __builtin_ctzll(current_chunk) + i * 64;
                        e.push_back(idx);
                        current_chunk &= (current_chunk - 1);
                    }
                }
                return e;
            }

            inline smallset_t smallset_init_empty()
            {
#ifdef __AVX2__
                return _mm256_setzero_si256();
#elif defined(__ARM_NEON)
                return {vdupq_n_u64(0), vdupq_n_u64(0)};
#else
                return smallset_t();
#endif
            }

            inline smallset_t smallset_init_full(const u32 len)
            {
#if !defined(__AVX2__) && !defined(__ARM_NEON)
                return smallset_t(len);
#endif
                // N must be in {256, 128, 64, 32, 16, 8}
                if (len == 256)
                {
#ifdef __AVX2__
                    return _mm256_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
#elif defined(__ARM_NEON)
                    return {vdupq_n_u64(0xFFFFFFFFFFFFFFFF), vdupq_n_u64(0xFFFFFFFFFFFFFFFF)};
#endif
                }
                else if (len == 128)
                {
#ifdef __AVX2__
                    return _mm256_set_epi64x(0, 0, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
#elif defined(__ARM_NEON)
                    return {vdupq_n_u64(0xFFFFFFFFFFFFFFFF), vdupq_n_u64(0)};
#endif
                }
                else if (len == 64)
                {
#ifdef __AVX2__
                    return _mm256_set_epi64x(0, 0, 0, 0xFFFFFFFFFFFFFFFF);
#elif defined(__ARM_NEON)
                    auto tmp = vdupq_n_u64(0);
                    return {vsetq_lane_u64(0xFFFFFFFFFFFFFFFF, tmp, 0), vdupq_n_u64(0)};
#endif
                }
                else if (len == 32)
                {
#ifdef __AVX2__
                    return _mm256_set_epi64x(0, 0, 0, 0xFFFFFFFF);
#elif defined(__ARM_NEON)
                    auto tmp = vdupq_n_u64(0);
                    return {(uint64x2_t)vsetq_lane_u32(0xFFFFFFFF, (uint32x4_t)tmp, 0), vdupq_n_u64(0)};
#endif
                }
                else if (len == 16)
                {
#ifdef __AVX2__
                    return _mm256_set_epi64x(0, 0, 0, 0xFFFF);
#elif defined(__ARM_NEON)
                    auto tmp = vdupq_n_u64(0);
                    return {(uint64x2_t)vsetq_lane_u16(0xFFFF, (uint16x8_t)tmp, 0), vdupq_n_u64(0)};
#endif
                }
                else if (len == 8)
                {
#ifdef __AVX2__
                    return _mm256_set_epi64x(0, 0, 0, 0xFF);
#elif defined(__ARM_NEON)
                    auto tmp = vdupq_n_u64(0);
                    return {(uint64x2_t)vsetq_lane_u8(0xFF, (uint8x16_t)tmp, 0), vdupq_n_u64(0)};
#endif
                }
                else
                {
#ifdef __AVX2__
                    return _mm256_set_epi64x(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
#elif defined(__ARM_NEON)
                    return {vdupq_n_u64(0xFFFFFFFFFFFFFFFF), vdupq_n_u64(0xFFFFFFFFFFFFFFFF)};
#endif
                }
            }

            inline smallset_t smallset_invert(const smallset_t& a, const u32 len)
            {
                const smallset_t b = smallset_init_full(len);
#ifdef __AVX2__
                return _mm256_xor_si256(a, b);
#elif defined(__ARM_NEON)
                return {veorq_u64(a.val[0], b.val[0]), veorq_u64(a.val[1], b.val[1])};
#else
                return (a ^ b);
#endif
            }

            inline smallset_t smallset_setminus(const smallset_t& a, const smallset_t& b, const u32 len)
            {
                const smallset_t b_not = smallset_invert(b, len);
                return smallset_intersect(a, b_not);
            }

            bool smallset_elm_is_in_set(const u8 e, const smallset_t& a)
            {
#if !defined(__AVX2__) && !defined(__ARM_NEON)
                return a.is_set(e);
#else
                smallset_t b = smallset_init_empty();
                b            = smallset_add_element(b, e);
                b            = smallset_intersect(a, b);
                return !smallset_is_empty(b);
#endif
            }
            // END OF SMALL SET //

            // state of the linear_representative algorithm
            typedef struct
            {
                std::vector<u8> A;
                std::vector<u8> B;
                std::vector<u8> R_S;
                smallset_t D_A;
                smallset_t D_B;
                smallset_t C_A;
                smallset_t C_B;
                smallset_t N_A;
                smallset_t N_B;
                smallset_t U_A;
                smallset_t U_B;
            } state_t;

            // lexicographically compare R_S and R_S_best
            bool is_greater(const std::vector<u8>& R_S, const std::vector<u8>& R_S_best, const u32 len)
            {
                if ((R_S_best[0] == 0) && (R_S_best[1] == 0))
                    return false;

                for (u32 x = 0; x < len; x++)
                {
                    // special case: R_S[x] not defined (=> 0) and R_S_best[x] = 0
                    // works out with this
                    if (R_S[x] > R_S_best[x])
                        return true;
                    if (R_S[x] < R_S_best[x])
                        return false;
                }
                // can happen if there are self equivalences (?)
                return false;
            }

            bool update_linear(std::vector<u8>& A, u8 new_x, const u32 len)
            {
                u8 new_y = A[new_x];
                for (u32 i = 1; i < len; i++)
                {
                    u8 e = A[i];
                    if (e == 0)
                        continue;
                    else if (A[new_x ^ i] == 0)
                        A[new_x ^ i] = e ^ new_y;
                    else if (A[new_x ^ i] != (e ^ new_y))
                    {
                        return false;
                    }
                }
                return true;
            }

            bool subroutine(const std::vector<u8>& S, const std::vector<u8>& S_inv, const state_t& state, std::vector<u8>& R_S_best, const u32 len)
            {
                std::vector<u8> A(state.A);
                std::vector<u8> B(state.B);
                std::vector<u8> R_S(state.R_S);

                smallset_t D_A = (state.D_A);
                smallset_t D_B = (state.D_B);
                smallset_t C_A = (state.C_A);
                smallset_t C_B = (state.C_B);
                smallset_t N_A = (state.N_A);
                smallset_t N_B = (state.N_B);
                smallset_t U_A = (state.U_A);
                smallset_t U_B = (state.U_B);

                while (!smallset_is_empty(N_A))
                {
                    u8 x = smallset_least_element(N_A);
                    u8 y = smallset_least_element(U_B);

                    B[y] = S[A[x]];
                    if (!update_linear(B, y, len))
                        return false;
                    smallset_t D_B_new = smallset_shift(D_B, y);
                    D_B                = smallset_union(D_B, D_B_new);
                    U_B                = smallset_setminus(U_B, D_B_new, len);

                    smallset_t SoA_N_A = smallset_init_empty();
                    for (u8 x : smallset_get_elements(N_A))
                    {
                        SoA_N_A = smallset_add_element(SoA_N_A, S[A[x]]);
                    }
                    smallset_t B_D_B_new = smallset_init_empty();
                    for (u8 d : smallset_get_elements(D_B_new))
                    {
                        B_D_B_new = smallset_add_element(B_D_B_new, B[d]);
                        if (smallset_elm_is_in_set(B[d], SoA_N_A))
                        {
                            C_B = smallset_add_element(C_B, d);
                        }
                        else
                        {
                            N_B = smallset_add_element(N_B, d);
                        }
                    }
                    smallset_t C_A_new = smallset_init_empty();
                    for (u8 x : smallset_get_elements(N_A))
                    {
                        if (smallset_elm_is_in_set(S[A[x]], B_D_B_new))
                        {
                            C_A_new = smallset_add_element(C_A_new, x);
                        }
                    }
                    C_A = smallset_union(C_A, C_A_new);
                    N_A = smallset_setminus(N_A, C_A_new, len);
                    for (u8 x : smallset_get_elements(C_A_new))
                    {
                        u8 y = 0;
                        for (u32 i = 0; i < len; i++)
                        {
                            if (B[i] == S[A[x]])
                            {
                                y = i;
                                break;
                            }
                        }
                        R_S[x] = y;
                    }
                    if (is_greater(R_S, R_S_best, len))
                    {
                        return false;
                    }

                    while (smallset_is_empty(N_A) && !smallset_is_empty(N_B))
                    {
                        u8 x = smallset_least_element(U_A);
                        u8 y = smallset_least_element(N_B);
                        A[x] = S_inv[B[y]];
                        if (!update_linear(A, x, len))
                        {
                            return false;
                        }
                        smallset_t D_A_new    = smallset_shift(D_A, x);
                        D_A                   = smallset_union(D_A, D_A_new);
                        U_A                   = smallset_setminus(U_A, D_A_new, len);
                        smallset_t SinvoB_N_B = smallset_init_empty();
                        for (u8 y : smallset_get_elements(N_B))
                        {
                            SinvoB_N_B = smallset_add_element(SinvoB_N_B, S_inv[B[y]]);
                        }
                        smallset_t A_D_A_new = smallset_init_empty();
                        for (u8 d : smallset_get_elements(D_A_new))
                        {
                            A_D_A_new = smallset_add_element(A_D_A_new, A[d]);
                            if (smallset_elm_is_in_set(A[d], SinvoB_N_B))
                            {
                                C_A = smallset_add_element(C_A, d);
                            }
                            else
                            {
                                N_A = smallset_add_element(N_A, d);
                            }
                        }
                        smallset_t C_B_new = smallset_init_empty();
                        for (u8 y : smallset_get_elements(N_B))
                        {
                            if (smallset_elm_is_in_set(S_inv[B[y]], A_D_A_new))
                            {
                                C_B_new = smallset_add_element(C_B_new, y);
                            }
                        }
                        C_B = smallset_union(C_B, C_B_new);
                        N_B = smallset_setminus(N_B, C_B_new, len);
                        for (u8 y : smallset_get_elements(C_B_new))
                        {
                            u8 x = 0;
                            for (u32 i = 0; i < len; i++)
                            {
                                if (A[i] == S_inv[B[y]])
                                {
                                    x = i;
                                    break;
                                }
                            }
                            R_S[x] = y;
                        }
                        if (is_greater(R_S, R_S_best, len))
                        {
                            return false;
                        }
                    }
                }
                if (smallset_is_empty(U_A) && smallset_is_empty(U_B))
                {
                    for (u32 i = 0; i < len; i++)
                    {
                        // new best
                        R_S_best[i] = R_S[i];
                    }
                    return true;
                }
                else
                {
                    u8 x               = smallset_least_element(U_A);
                    smallset_t D_A_new = smallset_shift(D_A, x);
                    U_A                = smallset_setminus(U_A, D_A_new, len);
                    D_A                = smallset_union(D_A, D_A_new);
                    N_A                = smallset_union(N_A, D_A_new);
                    bool flag          = false;
                    smallset_t Y       = smallset_init_full(len);
                    smallset_t A_set   = smallset_init_empty();
                    for (u32 i = 0; i < len; i++)
                    {
                        A_set = smallset_add_element(A_set, A[i]);
                    }
                    Y = smallset_setminus(Y, A_set, len);
                    for (u8 y : smallset_get_elements(Y))
                    {
                        std::vector<u8> A_next_guess(len);

                        for (u32 i = 0; i < len; i++)
                        {
                            A_next_guess[i] = A[i];
                        }
                        A_next_guess[x] = y;
                        if (!update_linear(A_next_guess, x, len))
                            continue;
                        state_t state_next;
                        state_next.A   = A_next_guess;
                        state_next.B   = B;
                        state_next.R_S = R_S;
                        state_next.D_A = D_A;
                        state_next.D_B = D_B;
                        state_next.C_A = C_A;
                        state_next.C_B = C_B;
                        state_next.N_A = N_A;
                        state_next.N_B = N_B;
                        state_next.U_A = U_A;
                        state_next.U_B = U_B;

                        if (subroutine(S, S_inv, state_next, R_S_best, len))
                        {
                            flag = true;
                        }
                    }

                    return flag;
                }
            }
        }    // namespace

        std::vector<u8> SBoxDatabase::compute_linear_representative(const std::vector<u8>& sbox)
        {
            u32 len = sbox.size();

            // variable for current best candidate
            std::vector<u8> R_S_best(len, 0);

            // invert sbox
            std::vector<u8> S_inv(len, 0);
            for (u32 x = 0; x < len; x++)
            {
                u8 y     = sbox[x];
                S_inv[y] = x;
            }

            // init the state of the algorithm
            state_t state;
            state.A   = std::vector<u8>(len, 0);
            state.B   = std::vector<u8>(len, 0);
            state.R_S = std::vector<u8>(len, 0);

            state.D_A = smallset_add_element(smallset_init_empty(), 0);
            state.D_B = smallset_add_element(smallset_init_empty(), 0);

            state.C_A = smallset_init_empty();
            state.C_B = smallset_init_empty();

            state.N_A = smallset_add_element(smallset_init_empty(), 0);
            state.N_B = smallset_add_element(smallset_init_empty(), 0);

            state.U_A = smallset_setminus(smallset_init_full(len), state.D_A, len);
            state.U_B = smallset_setminus(smallset_init_full(len), state.D_A, len);

            // init in special case S[0] == 0
            if (sbox[0] == 0)
            {
                state.C_A = smallset_add_element(smallset_init_empty(), 0);
                state.C_B = smallset_add_element(smallset_init_empty(), 0);

                state.N_A = smallset_init_empty();
                state.N_B = smallset_init_empty();
            }

            // compute linear representative recursively
            subroutine(sbox, S_inv, state, R_S_best, len);

            return R_S_best;
        }
    }    // namespace hawkeye
}    // namespace hal
