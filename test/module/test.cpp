#include <iostream>

#include "flecs.h"
#include "doctest.h"
#include "cpp/simple.h"

TEST_CASE("flecs module tests") {
    flecs::world ecs;

    SimpleModuleImport(ecs);

    SUBCASE("use module components (C++)") {
        // Create entity with imported components
        flecs::entity e = ecs.entity()
                .set<simple::Position>({10, 20})
                .set<simple::Velocity>({1, 1});

        e.get([](const simple::Position& p) {
            //std::cout << "p = {" << p.x << ", " << p.y << "} (get)\n";
            CHECK(p.x == doctest::Approx(10.0));
            CHECK(p.y == doctest::Approx(20.0));
        });

        // Call progress which runs imported Move system
        ecs.progress();

        // Use component from module in operation
        e.get([](const simple::Position& p) {
            //std::cout << "p = {" << p.x << ", " << p.y << "} (get)\n";
            CHECK(p.x == doctest::Approx(11.0));
            CHECK(p.y == doctest::Approx(21.0));
        });
    }

    SUBCASE("list module components (C++)") {

    }
}
