#include <iostream>
#include <string>
#include <algorithm>

#include "flecs.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include "simple.h"

#ifndef MODULE_TEST_STATIC
#   if defined(_WIN32)
#include <windows.h>
#   else   // Unix-like
#include <dlfcn.h>
#   endif
#endif

typedef ecs_id_t (*ImportModuleFnPtr)(
        ecs_world_t *world);

#if defined(_WIN32)
static const char* ModuleLibraryName = "module_simple.dll";
#elif __APPLE__
static const char* ModuleLibraryName = "libmodule_simple.dylib";
#else
static const char* ModuleLibraryName = "libmodule_simple.so";
#endif
static const char* ModuleImportProcName = "SimpleModuleImport";

static const char* EntityName = "Entity";
static constexpr ecs_ftime_t StepSize = 1.0;

std::string sBinaryName;

struct Vec2d {
    double x, y;
};

template <class T>
static inline T UnpackStruct(const void* src)
{
    T result {};
    if (src) {
        memcpy(&result, src, sizeof(T));
    }
    return result;
}

#ifndef MODULE_TEST_STATIC
ImportModuleFnPtr LoadModule(const char* libraryName, const char* initProcName)
{
    ImportModuleFnPtr fnPtr {};
#if defined(_WIN32)
    HMODULE dll = LoadLibrary(libraryName);
    if (!dll) {
        DWORD error_value = GetLastError();
        if (error_value == 0) {
            return NULL;
        }

        LPVOID lpMsgBuf;

        FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                error_value,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0, NULL );

        fprintf(stderr, "Failed to load module '%s': %s\n", libraryName, lpMsgBuf);
        LocalFree(lpMsgBuf);
        abort();
    }
    fnPtr = reinterpret_cast<ImportModuleFnPtr>(GetProcAddress(dll, initProcName));
#else
    void* dl = dlopen(libraryName, RTLD_NOW);
    if (!dl) {
        char *err = dlerror();
        fprintf(stderr, "Failed to load module '%s': %s\n", libraryName, err);
        abort();
    }
    fnPtr = reinterpret_cast<ImportModuleFnPtr>(dlsym(dl, initProcName));
#endif
    if (!fnPtr) {
        fprintf(stderr, "Failed to load module '%s': symbol not found: %s\n", libraryName, initProcName);
        abort();
    }
    return fnPtr;
}
#endif

TEST_CASE("flecs module tests") {
    using namespace doctest;

    flecs::world ecs;

    ecs_id_t modId;
#ifdef MODULE_TEST_STATIC
    modId = SimpleModuleImport(ecs);
#else
    std::string moduleLibraryName {ModuleLibraryName };
    std::string moduleLibraryPath {moduleLibraryName };

    auto binaryPath = sBinaryName;
    std::cout << "binaryPath: " << binaryPath << std::endl;
    auto binaryDirSepIdx = binaryPath.rfind('/');
    if (binaryDirSepIdx != std::string::npos) {
        moduleLibraryPath = binaryPath.substr(0, binaryDirSepIdx+1) + moduleLibraryName;
    }

    auto importFn = LoadModule(moduleLibraryPath.c_str(), ModuleImportProcName);
    modId = importFn(ecs);
#endif
    flecs::entity module(ecs, modId);

    auto Position = module.lookup("Position");
    auto Velocity = module.lookup("Velocity");
    SUBCASE("creates components") {
        CHECK(Position);
        CHECK(Velocity);
    }

    SUBCASE("global entity") {
        SUBCASE("not created during module load") {
            CHECK(!ecs.lookup(EntityName));
        }

        ecs.set_time_scale(0);
        ecs.progress(StepSize);

        auto entity = ecs.lookup(EntityName);
        SUBCASE("created during first update") {
            CHECK(entity);
            CHECK(entity.is_valid());
            CHECK(entity.is_alive());
        }

        SUBCASE("matching initial values") {
            auto p = UnpackStruct<Vec2d>(entity.get(Position));
            CHECK(p.x == doctest::Approx(10.0));
            CHECK(p.y == doctest::Approx(20.0));

            auto v = UnpackStruct<Vec2d>(entity.get(Velocity));
            CHECK(v.x == doctest::Approx(1.0));
            CHECK(v.y == doctest::Approx(1.0));
        }

        ecs.set_time_scale(1);
        ecs.progress(StepSize);

        SUBCASE("matching updated values") {
            auto p = UnpackStruct<Vec2d>(entity.get(Position));
            CHECK(p.x == doctest::Approx(11.0));
            CHECK(p.y == doctest::Approx(21.0));
        }
    }
}

int main(int argc, char** argv) {

    sBinaryName = argv[0];
    std::replace( sBinaryName.begin(), sBinaryName.end(), '\\', '/');
    return doctest::Context(argc, argv).run();
}
