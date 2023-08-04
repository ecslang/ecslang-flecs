#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/utsname.h>
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

    std::cout << "Number of Processors: " << sysInfo.dwNumberOfProcessors << std::endl;

    DWORD bufSize = 0;
    GetLogicalProcessorInformation(0, &bufSize);
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION* buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)malloc(bufSize);
    GetLogicalProcessorInformation(buffer, &bufSize);

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

            cache.lineSize = cacheDesc.LineSize;
            cache.size = cacheDesc.Size;

            caches.push_back(cache);
        }
    }

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

    std::cout << "\nCache Details:\n";
    std::cout << "  CPU-Mask  | Level-Type | Associativity | Line Size (bytes) | Size (bytes)\n";
    std::cout << "------------+------------+---------------+-------------------+---------------\n";
    for (const auto& cache : caches) {
        std::cout << std::setw(11) << cache.getProcessorMaskStr() << " | "
                << std::setw(10) << cache.getLevelTypeStr() << " | "
                << std::setw(13) << (cache.associativity == -1 ? "Fully Associative" : std::to_string(cache.associativity)) << " | "
                << std::setw(17) << cache.lineSize << " | "
                << std::setw(12) << cache.size << '\n';
    }

    return 0;
}
