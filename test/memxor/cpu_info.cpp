#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <new>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/utsname.h>
#endif

#ifdef _WIN32
#include <intrin.h>
#elif defined(__GNUC__) || defined(__clang__)
#include <immintrin.h>
#endif

#ifdef __cpp_lib_hardware_interference_size
    using std::hardware_constructive_interference_size;
    using std::hardware_destructive_interference_size;
#else
    // 64 bytes on x86-64 │ L1_CACHE_BYTES │ L1_CACHE_SHIFT │ __cacheline_aligned │ ...
    constexpr std::size_t hardware_constructive_interference_size = 64;
    constexpr std::size_t hardware_destructive_interference_size = 64;
#endif

struct CacheDetails {
    std::string getProcessorMaskStr() const
    {
        std::vector<int> processors;
        for (int i = 0; i < sizeof(processorMask) * 8; ++i) {
            if (processorMask & (1ULL << i)) {
                processors.push_back(i);
            }
        }

        std::string maskString;
        if (level == "L1" || type == "Data" || type == "Instruction" || processors.size() <= 2) {
            // For L1, Data, or Instruction caches, list individual processors
            for (int i = 0; i < processors.size(); ++i) {
                if (i > 0) maskString += ",";
                maskString += std::to_string(processors[i]);
            }
        } else {
            // For other cache levels/types, summarize with ranges
            for (int i = 0; i < processors.size();) {
                int startRange = processors[i];
                int endRange = startRange;
                while (i + 1 < processors.size() && processors[i + 1] == processors[i] + 1) {
                    endRange = processors[i + 1];
                    ++i;
                }
                if (maskString.length() > 0) maskString += ", ";
                maskString += "(";
                maskString += (startRange == endRange) ? std::to_string(startRange) : std::to_string(startRange) + ".." + std::to_string(endRange);
                maskString += ")";
                ++i;
            }
        }

        return maskString;
    }

    std::string getLevelTypeStr() const {
        std::string levelType = level;
        if (type == "Data") levelType += "-D";
        else if (type == "Instruction") levelType += "-I";
        return levelType;
    }

    std::string getSizeInCacheLines() const {
        if (lineSize == 0) {
            return "N/A";
        }
        int cacheLines = size / lineSize;
        return std::to_string(cacheLines) + " x " + std::to_string(lineSize) + " bytes";
    }

    uint64_t processorMask;
    int numaNodeNumber;
    bool sharedFunctionalUnits;
    std::string level;
    std::string type;
    int associativity;
    int lineSize;
    int size;
};

int main() {
    std::vector<CacheDetails> caches;

    bool supportsAVX = false;
    bool supportsAVX2 = false;
    bool supportsAVX512 = false;

#if defined(_WIN32)
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    supportsAVX = cpuInfo[2] & (1 << 28);

    __cpuidex(cpuInfo, 7, 0);
    supportsAVX2 = cpuInfo[1] & (1 << 5);
    supportsAVX512 = cpuInfo[1] & (1 << 16);
#elif defined(__GNUC__) || defined(__clang__)
    supportsAVX   = __builtin_cpu_supports("avx");
    supportsAVX2  = __builtin_cpu_supports("avx2");
    supportsAVX512 = __builtin_cpu_supports("avx512f");
#endif

#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    std::cout << "Processor Architecture: ";
    switch (sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            std::cout << "x64 (AMD or Intel)" << std::endl;
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            std::cout << "ARM" << std::endl;
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
            std::cout << "Intel Itanium-based" << std::endl;
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            std::cout << "x86" << std::endl;
            break;
        default:
            std::cout << "Unknown" << std::endl;
            break;
    }

    DWORD bufSize = 0;
    GetLogicalProcessorInformation(0, &bufSize);
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION* buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)malloc(bufSize);
    GetLogicalProcessorInformation(buffer, &bufSize);

    constexpr size_t SIZE_INVALID = std::numeric_limits<size_t>::max();
    size_t cacheLineSize = hardware_destructive_interference_size;
    size_t cacheLineSizeL1D = SIZE_INVALID;
    size_t cacheLineSizeL2 = SIZE_INVALID; // Using standard constant for cache line size

    for (DWORD offset = 0; offset < bufSize; offset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION)) {
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION cpuInfo = buffer[offset / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION)];
        if (buffer[offset / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION)].Relationship == RelationCache) {
            CACHE_DESCRIPTOR cacheDesc = cpuInfo.Cache;
            
            CacheDetails cache;

            cache.processorMask = cpuInfo.ProcessorMask;
            cache.numaNodeNumber = (cpuInfo.Relationship == RelationNumaNode) ? cpuInfo.NumaNode.NodeNumber : -1;
            cache.sharedFunctionalUnits = (cpuInfo.Relationship == RelationProcessorCore && cpuInfo.ProcessorCore.Flags == 1);

            switch (cacheDesc.Level) {
                case 1: cache.level = "L1"; break;
                case 2: cache.level = "L2"; break;
                case 3: cache.level = "L3"; break;
                default: cache.level = "Unknown"; break;
            }

            switch (cacheDesc.Type) {
                case CacheUnified: cache.type = "Unified"; break;
                case CacheInstruction: cache.type = "Instruction"; break;
                case CacheData: cache.type = "Data"; break;
                case CacheTrace: cache.type = "Trace"; break;
                default: cache.type = "Unknown"; break;
            }

            if (cacheDesc.Associativity == 0xFF) {
                cache.associativity = -1; // Will print "Fully Associative"
            } else {
                cache.associativity = cacheDesc.Associativity;
            }

            DWORD lineSize = cacheDesc.LineSize;
            cache.lineSize = lineSize;
            cache.size = cacheDesc.Size;

            if (lineSize > 0) {
                if (cacheDesc.Level == 1 && cacheDesc.Type == CacheData && cacheDesc.LineSize < cacheLineSizeL1D) {
                    cacheLineSizeL1D = cacheDesc.LineSize;
                } else if (cacheDesc.Level == 2 && cacheDesc.Type == CacheUnified && cacheDesc.LineSize < cacheLineSizeL2) {
                    cacheLineSizeL2 = cacheDesc.LineSize;
                }
            }
            caches.push_back(cache);
        }
    }

    if (cacheLineSizeL1D != SIZE_INVALID) {
        cacheLineSize = cacheLineSizeL1D;
    } else if (cacheLineSizeL2 != SIZE_INVALID) {
        cacheLineSize = cacheLineSizeL2;
    }

    size_t cacheLinesPerPage = sysInfo.dwPageSize / cacheLineSize;
    size_t cacheLinesPerPageL1D = (cacheLineSizeL1D != SIZE_INVALID) ? sysInfo.dwPageSize / cacheLineSizeL1D : SIZE_INVALID;

    std::cout << "Number of Processors: " << sysInfo.dwNumberOfProcessors << std::endl;
    std::cout << "Cache Line: " << cacheLineSize << " bytes" << std::endl;
    if (cacheLineSizeL1D != SIZE_INVALID) {
        std::cout << "Cache Line (L1D): " << cacheLineSizeL1D << " bytes" << std::endl;
    }
    if (cacheLineSizeL2 != SIZE_INVALID) {
        std::cout << "Cache Line (L2): " << cacheLineSizeL2 << " bytes" << std::endl;
    }
    if (cacheLinesPerPageL1D != SIZE_INVALID) {
        std::cout << "Page Size: " << sysInfo.dwPageSize << " bytes (" << cacheLinesPerPageL1D << " x L1D cache line)" << std::endl;
    }
    else {
        std::cout << "Page Size: " << sysInfo.dwPageSize << " bytes (" << cacheLinesPerPage << " x cache line)" << std::endl;
    }
    std::cout << "Minimum Application Address: " << sysInfo.lpMinimumApplicationAddress << std::endl;
    std::cout << "Maximum Application Address: " << sysInfo.lpMaximumApplicationAddress << std::endl;
    std::cout << "Active Processor Mask: " << sysInfo.dwActiveProcessorMask << std::endl;

    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);

    std::cout << "Total Physical Memory: " << memInfo.ullTotalPhys << " bytes" << std::endl;
    std::cout << "Available Physical Memory: " << memInfo.ullAvailPhys << " bytes" << std::endl;
    std::cout << "Total Virtual Memory: " << memInfo.ullTotalVirtual << " bytes" << std::endl;
    std::cout << "Available Virtual Memory: " << memInfo.ullAvailVirtual << " bytes" << std::endl;

    free(buffer);
#else
    std::cout << "Number of Processors: " << sysconf(_SC_NPROCESSORS_ONLN) << std::endl;

    struct utsname uts;
    if (uname(&uts) == 0) {
        std::cout << "Processor Type: " << uts.machine << std::endl;
    } else {
        std::cout << "Failed to determine processor type." << std::endl;
    }

    // Note: POSIX doesn't provide an easy way to query CPU cache sizes
#endif

    if (!caches.empty()) {
        std::cout << "\nCache Details:\n";
        std::cout << "  CPU-Mask  | Level-Type | Associativity |    Cache Size     |     Cache Lines    \n";
        std::cout << "------------+------------+---------------+-------------------+--------------------\n";
        for (const auto& cache : caches) {
            std::cout << std::setw(11) << cache.getProcessorMaskStr() << " | "
                    << std::setw(10) << cache.getLevelTypeStr() << " | "
                    << std::setw(13) << (cache.associativity == -1 ? "Fully Associative" : std::to_string(cache.associativity)) << " | "
                    << std::setw(17) << (std::to_string(cache.size) + " bytes") << " | "
                    << std::setw(19) << cache.getSizeInCacheLines() << "\n";
        }
    }

    if (supportsAVX || supportsAVX2 || supportsAVX512) {
        std::cout << "\nRuntime Supported SIMD Intrinsics:\n";

        if (supportsAVX) {
            std::cout << "- AVX\n";
        }

        if (supportsAVX2) {
            std::cout << "- AVX2\n";
        }

        if (supportsAVX512) {
            std::cout << "- AVX-512\n";
        }
    }
    return 0;
}
