#include "flecs.h"
#include "doctest.h"

struct Vec2d {
    double x, y;
};

TEST_CASE("flecs system tests") {
    flecs::world ecs;

    SUBCASE("create and destroy entity") {
        auto e = ecs.entity();
        CHECK(e.is_alive());

        e.destruct();
        CHECK(!e.is_alive());
    }

    SUBCASE("value reflection") {
        struct Position {
            double x, y;
        };

        auto position = ecs.component<Position>("Position")
            .member<double>("x")
            .member<double>("y");

        auto e = ecs.entity();
        e.set<Position>({10.0, 20.0});

        auto *pPos = e.get(position);
        REQUIRE(pPos);
        CHECK_EQ(ecs.to_expr(position, pPos).c_str(), doctest::String("{x: 10, y: 20}"));
    }
}
