#include <iostream>

#include "flecs.h"
#include "doctest.h"
#include "simple.h"

static const char* EntityName = "Entity";
static constexpr ecs_ftime_t StepSize = 1.0;

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

TEST_CASE("flecs module tests") {
    flecs::world ecs;

    auto modId = SimpleModuleImport(ecs);
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
