#include "simple.h"

namespace simple {

module::module(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple.module)
    ecs.module<module>();

    // All contents of the module are created inside the module's namespace, so
    // the Position component will be created as simple.module.Position

    // Component registration is optional, however by registering components
    // inside the module constructor, they will be created inside the scope
    // of the module.
    ecs.component<Position>()
        .member<double>("x")
        .member<double>("y");
    ecs.component<Velocity>()
        .member<double>("x")
        .member<double>("y");

    ecs.system<Position, const Velocity>("Move")
        .each([](flecs::iter& it, size_t, Position& p, const Velocity& v) {
            p.x += v.x * it.delta_time();
            p.y += v.y * it.delta_time();
        });

    // Startup system
    ecs.system("OnStart")
        .kind(flecs::OnStart)
        .iter([](flecs::iter& it) {
            it.world()
                .entity("Entity")
                .set<simple::Position>({10, 20})
                .set<simple::Velocity>({1, 1});
        });
}

}

ecs_entity_t SimpleModuleImport(ecs_world_t *world)
{
    return flecs::world(world)
        .import<simple::module>();
}
