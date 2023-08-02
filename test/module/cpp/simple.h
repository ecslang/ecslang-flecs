#pragma once

#if defined(_WIN32) && defined(MODULE_SHARED)
#   ifdef MODULE_BUILD
#       define MODULE_EXPORT __declspec(dllexport)
#   else
#       define MODULE_EXPORT __declspec(dllimport)
#   endif
#elif defined(MODULE_SHARED) && defined(MODULE_BUILD)
#   define MODULE_EXPORT __attribute__((__visibility__("default")))
#else
#   define MODULE_EXPORT
#endif

#include "flecs.h"

#ifdef __cplusplus
extern "C" {
#endif

namespace simple {

struct Position {
    double x, y;
};

struct Velocity {
    double x, y;
};

struct module {
    module(flecs::world& world); // Ctor that loads the module
};

}

MODULE_EXPORT ecs_entity_t SimpleModuleImport(ecs_world_t *world);

#ifdef __cplusplus
}
#endif
