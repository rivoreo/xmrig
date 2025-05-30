#include <stddef.h>

#if defined __i386__ || defined __amd64__ || defined _M_IX86 || defined _M_AMD64 || defined __x86_64 || defined __x86_64__
#include "BasicCpuInfo_x86.cpp"
#elif defined __arm__ || defined __ARMEL__ || defined __arm64__ || defined __aarch64__ || defined __AARCH64EL__
#include "BasicCpuInfo_arm.cpp"
#else
#include "BasicCpuInfo_unknown.cpp"
#endif


#include "backend/cpu/platform/BasicCpuInfo.h"
#include "crypto/common/Assembly.h"


#define VENDOR_ID                  (0)
#define PROCESSOR_INFO             (1)
#define EXTENDED_FEATURES          (7)
#define PROCESSOR_BRAND_STRING_1   (0x80000002)
#define PROCESSOR_BRAND_STRING_2   (0x80000003)
#define PROCESSOR_BRAND_STRING_3   (0x80000004)

#define EAX_Reg  (0)
#define EBX_Reg  (1)
#define ECX_Reg  (2)
#define EDX_Reg  (3)


namespace xmrig {


static inline void cpuid(uint32_t level, int32_t output[4])
{
    memset(output, 0, sizeof(int32_t) * 4);

#   ifdef _MSC_VER
    __cpuid(output, static_cast<int>(level));
#   else
    __cpuid_count(level, 0, output[0], output[1], output[2], output[3]);
#   endif
}


static void cpu_brand_string(char out[64 + 6]) {
    int32_t cpu_info[4] = { 0 };
    char buf[64]        = { 0 };

    cpuid(VENDOR_ID, cpu_info);

    if (cpu_info[EAX_Reg] >= 4) {
        for (uint32_t i = 0; i < 4; i++) {
            cpuid(0x80000002 + i, cpu_info);
            memcpy(buf + (i * 16), cpu_info, sizeof(cpu_info));
        }
    }

    size_t pos        = 0;
    const size_t size = strlen(buf);

    for (size_t i = 0; i < size; ++i) {
        if (buf[i] == ' ' && ((pos > 0 && out[pos - 1] == ' ') || pos == 0)) {
            continue;
        }

        out[pos++] = buf[i];
    }

    if (pos > 0 && out[pos - 1] == ' ') {
        out[pos - 1] = '\0';
    }
}


static bool has_feature(uint32_t level, uint32_t reg, int32_t bit)
{
    int32_t cpu_info[4] = { 0 };
    cpuid(level, cpu_info);

    return (cpu_info[reg] & bit) != 0;
}


static inline int32_t get_masked(int32_t val, int32_t h, int32_t l)
{
    val &= (0x7FFFFFFF >> (31 - (h - l))) << l;
    return val >> l;
}


static inline bool has_aes_ni()
{
    return has_feature(PROCESSOR_INFO, ECX_Reg, bit_AES);
}


static inline bool has_avx2()
{
    return has_feature(EXTENDED_FEATURES, EBX_Reg, bit_AVX2) && has_feature(PROCESSOR_INFO, ECX_Reg, bit_OSXSAVE);
}


} // namespace xmrig


xmrig::BasicCpuInfo::BasicCpuInfo() :
    m_brand(),
    m_threads(std::thread::hardware_concurrency()),
    m_assembly(Assembly::NONE),
    m_aes(has_aes_ni()),
    m_avx2(has_avx2())
{
    cpu_brand_string(m_brand);

#   ifdef XMRIG_FEATURE_ASM
    if (hasAES()) {
        char vendor[13] = { 0 };
        int32_t data[4] = { 0 };

        cpuid(VENDOR_ID, data);

        memcpy(vendor + 0, &data[1], 4);
        memcpy(vendor + 4, &data[3], 4);
        memcpy(vendor + 8, &data[2], 4);

        if (memcmp(vendor, "AuthenticAMD", 12) == 0) {
            cpuid(PROCESSOR_INFO, data);
            const int32_t family = get_masked(data[EAX_Reg], 12, 8) + get_masked(data[EAX_Reg], 28, 20);

            m_assembly = family >= 23 ? Assembly::RYZEN : Assembly::BULLDOZER;
        }
        else {
            m_assembly = Assembly::INTEL;
        }
    }
#   endif
}


const char *xmrig::BasicCpuInfo::backend() const
{
    return "basic";
}


xmrig::CpuThreads xmrig::BasicCpuInfo::threads(const Algorithm &algorithm, uint32_t limit) const
{
    const size_t count = std::thread::hardware_concurrency();

    if (count == 1) {
        return 1;
    }

#   ifdef XMRIG_ALGO_CN_GPU
    if (algorithm == Algorithm::CN_GPU) {
        return count;
    }
#   endif

#   ifdef XMRIG_ALGO_CN_LITE
    if (algorithm.family() == Algorithm::CN_LITE) {
        return CpuThreads(count, 1);
    }
#   endif

#   ifdef XMRIG_ALGO_CN_PICO
    if (algorithm.family() == Algorithm::CN_PICO) {
        return CpuThreads(count, 2);
    }
#   endif

#   ifdef XMRIG_ALGO_CN_HEAVY
    if (algorithm.family() == Algorithm::CN_HEAVY) {
        return CpuThreads(std::max<size_t>(count / 4, 1), 1);
    }
#   endif

#   ifdef XMRIG_ALGO_RANDOMX
    if (algorithm.family() == Algorithm::RANDOM_X) {
        if (algorithm == Algorithm::RX_WOW) {
            return count;
        }

        return std::max<size_t>(count / 2, 1);
    }
#   endif

#   ifdef XMRIG_ALGO_ARGON2
    if (algorithm.family() == Algorithm::ARGON2) {
        return count;
    }
#   endif

    return CpuThreads(std::max<size_t>(count / 2, 1), 1);
}
