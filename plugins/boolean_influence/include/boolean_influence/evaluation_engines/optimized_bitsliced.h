
#include <string>

namespace hal
{
    namespace boolean_influence
    {
        namespace
        {
            const std::string optimized_bitsliced_function = R"(
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(_MSC_VER)
  #include <intrin.h>   // for __popcnt64
#endif

#define INPUT_SIZE  <INPUT_SIZE>
#define K           <K>
#define NUM_TRIALS  <NUM_TRIALS>

<C_FUNCTION>

// ---- small inline helper control ----
#if defined(__GNUC__) || defined(__clang__)
  #define FORCE_INLINE __attribute__((always_inline)) inline
#else
  #define FORCE_INLINE inline
#endif

/* xoshiro256** */
static uint64_t S[4] = {
    0x0123456789abcdefULL, 0xfedcba9876543210ULL,
    0x0f1e2d3c4b5a6978ULL, 0x8877665544332211ULL
};
static FORCE_INLINE uint64_t rotl64(uint64_t x, int k){ return (x<<k)|(x>>(64-k)); }
static FORCE_INLINE uint64_t xoshiro256ss(void){
    const uint64_t r = rotl64(S[1]*5u,7)*9u, t = S[1]<<17;
    S[2]^=S[0]; S[3]^=S[1]; S[1]^=S[2]; S[0]^=S[3]; S[2]^=t; S[3]=rotl64(S[3],45);
    return r;
}
static FORCE_INLINE uint64_t splitmix64(uint64_t *x){
    uint64_t z = (*x += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z>>30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z>>27)) * 0x94D049BB133111EBULL;
    return z ^ (z>>31);
}
static FORCE_INLINE void seed_xoshiro256(uint64_t seed){
    for(int i=0;i<4;++i) S[i]=splitmix64(&seed);
    if(!(S[0]|S[1]|S[2]|S[3])) S[0]=1;
}

/* coarse-bias mask via OR: p = 1 - 2^-(K+1) */
static FORCE_INLINE uint64_t bernoulli_mask_or(void){
    uint64_t m = xoshiro256ss();
#if (K>=1)
    m |= xoshiro256ss();
#endif
#if (K>=2)
    m |= xoshiro256ss();
#endif
#if (K>=3)
    m |= xoshiro256ss();
#endif
#if (K>=4)
    m |= xoshiro256ss();
#endif
#if (K>=5)
    m |= xoshiro256ss();
#endif
#if (K>=6)
    m |= xoshiro256ss();
#endif
#if (K>=7)
    m |= xoshiro256ss();
#endif
    return m;
}

static FORCE_INLINE void build_masks(uint64_t *vmask){
    for (int i=0;i<INPUT_SIZE;++i) vmask[i]=bernoulli_mask_or();
}

static FORCE_INLINE unsigned popcnt64_u64(uint64_t x){
#if defined(__GNUC__) || defined(__clang__)
    return (unsigned)__builtin_popcountll(x);
#elif defined(_MSC_VER)
    return (unsigned)__popcnt64(x);
#else
    x -= (x>>1) & 0x5555555555555555ULL;
    x  = (x & 0x3333333333333333ULL) + ((x>>2)&0x3333333333333333ULL);
    x  = (x + (x>>4)) & 0x0F0F0F0F0F0F0F0FULL;
    return (unsigned)((x*0x0101010101010101ULL)>>56);
#endif
}

enum { BATCHES = (NUM_TRIALS/64) };
enum { TAIL    = (NUM_TRIALS%64) };

#if (TAIL>0)
static FORCE_INLINE uint64_t tail_mask_const(void){
    return (TAIL==0)?0ULL:((1ULL<<TAIL)-1ULL);
}
#endif

static FORCE_INLINE unsigned long long run_engine(unsigned b_index){
    // Diversify streams by b_index to avoid identical sequences across runs.
    uint64_t seed = 0xA5A5A5A5A5A5A5A5ULL
                  ^ (uint64_t)INPUT_SIZE ^ (uint64_t)K ^ (uint64_t)NUM_TRIALS
                  ^ ((uint64_t)b_index * 0x9E3779B97F4A7C15ULL);
    seed_xoshiro256(seed);

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    _Alignas(64)
#endif
    uint64_t vmask[INPUT_SIZE];

    unsigned long long total=0;

    for (int batch=0; batch<BATCHES; ++batch){
        build_masks(vmask);

        vmask[b_index]=~0ULL; 
        const uint64_t r1 = func64(vmask);

        vmask[b_index]=0ULL;  
        const uint64_t r2 = func64(vmask);

        total += (unsigned long long)popcnt64_u64(r1^r2);
    }

#if (TAIL>0)
    {
        build_masks(vmask);

        vmask[b_index]=~0ULL; 
        const uint64_t r1 = func64(vmask);

        vmask[b_index]=0ULL;  
        const uint64_t r2 = func64(vmask);

        total += (unsigned long long)popcnt64_u64((r1^r2) & tail_mask_const());
    }
#endif
    return total;
}

int main(int argc, char** argv){
    if (argc!=2) return 1;
    unsigned long long b = strtoull(argv[1],0,10);
    if (b >= (unsigned long long)INPUT_SIZE) return 2;
    unsigned long long out = run_engine((unsigned)b);
    printf("%llu\n", out);
    return 0;
})";
        }
    }    // namespace boolean_influence
}    // namespace hal
