#include "flecs.h"
#include "doctest/doctest.h"

TEST_CASE("flecs system tests") {
    flecs::world world;

    SUBCASE("create and destroy entity") {
        auto e = world.entity();
        CHECK(e.is_alive());

        e.destruct();
        CHECK(!e.is_alive());
    }

}
