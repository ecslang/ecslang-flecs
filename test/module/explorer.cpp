#include <iostream>

#include "flecs.h"
#include "simple.h"

void iterate_components(flecs::entity e) {
    // 1. The easiest way to print the components is to use type::str
    std::cout << e.type().str() << "\n\n";

    // 2. To get individual component ids, use entity::each
    std::int32_t i = 0;
    e.each([&](flecs::id id) {
        std::cout << i++ << ": " << id.str() << "\n";
    });
    std::cout << "\n";

    // 3. we can also inspect and print the ids in our own way. This is a
    // bit more complicated as we need to handle the edge cases of what can be
    // encoded in an id, but provides the most flexibility.
    i = 0;
    e.each([&](flecs::id id) {
        std::cout << i++ << ": ";

        if (id.is_pair()) {
            // If id is a pair, extract & print both parts of the pair
            flecs::entity rel = id.first();
            flecs::entity tgt = id.second();
            std::cout << "rel: " << rel.name() << ", " << "tgt: " << tgt.name();
        } else {
            // Id contains a regular entity. Strip role before printing.
            flecs::entity comp = id.entity();
            std::cout << "entity: " << comp.name();
        }

        std::cout << "\n";
    });

    std::cout << "\n\n";
}

void iterate_tree(flecs::entity e) {
    // Print hierarchical name of entity & the entity type
    std::cout << e.path() << " [" << e.type().str() << "]\n";

    // Iterate children recursively
    e.children([&](flecs::entity child) {
        iterate_tree(child);
    });
}

int main(int argc, char *argv[]) {
    // Passing in the command line arguments will allow the explorer to display
    // the application name.
    flecs::world world(argc, argv);

    world.import<flecs::monitor>(); // Collect statistics periodically

    auto modId = SimpleModuleImport(world);
    flecs::entity module(world, modId);
    iterate_components(module);
    iterate_tree(module);

    // Run application with REST interface. When the application is running,
    // navigate to https://flecs.dev/explorer to inspect it!
    //
    // See docs/RestApi.md#explorer for more information.
    return world.app()
        .enable_rest()
        .run();
}