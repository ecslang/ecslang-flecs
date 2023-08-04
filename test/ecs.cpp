#include <iostream>
#include <deque>
#include <vector>

#include "flecs.h"
#include "doctest.h"

struct Vec2d {
    double x, y;
};

struct Position {
    double x, y;
};

struct Rotation {
    double deg;
};

bool DEBUG_LOGGING = true;

void debug_log_evt(flecs::iter& it)
{
    if (!DEBUG_LOGGING) {
        return;
    }

//    it.
    //it.to_json().print();

    flecs::entity_to_json_desc_t evtDesc;

    auto evtId = it.event_id();

    std::cout << "event_id: " << evtId << std::endl;
    std::cout << "entity(event_id): " <<
        it.world().entity(evtId).to_json(&evtDesc) << std::endl;
    std::cout << "component(event_id): " <<
        it.world().component(evtId).to_json(&evtDesc) << std::endl;

    std::cout << "it.event: " << it.event().to_json(&evtDesc) << std::endl;

    flecs::entity_to_json_desc_t entityDesc;
    entityDesc.serialize_path = true;
    entityDesc.serialize_values = true;
    entityDesc.serialize_id_labels = true;

    for (auto i : it) {
        if (it.event() == flecs::OnAdd) {
            // No assumptions about the component value should be made here. If
            // a ctor for the component was registered it will be called before
            // the EcsOnAdd event, but a value assigned by set won't be visible.
            std::cout << " - OnAdd: " << it.event_id().str() << ": "
                      << it.entity(i).name() << "\n";
        } else {
            // EcsOnSet or EcsOnRemove event
            std::cout << " - " << it.event().name() << ": "
                      << it.event_id().str() << ": "
                      << it.entity(i).name()
                      << ": {" << it.entity(i).to_json(&entityDesc) << "}\n";
        }
    }
}

TEST_CASE("flecs system tests") {
    flecs::world ecs;

    auto position = ecs.component<Position>("Position")
            .member<double>("x")
            .member<double>("y");

    auto rotation = ecs.component<Rotation>("Rotation")
            .member<double>("deg");

    SUBCASE("create and destroy entity") {
        auto e = ecs.entity();
        CHECK(e.is_alive());

        e.destruct();
        CHECK(!e.is_alive());
    }

    SUBCASE("value reflection") {
        auto e = ecs.entity();
        e.set<Position>({10.0, 20.0});
        e.set<Rotation>({45.0});

        auto *ptr = e.get(position);
        REQUIRE(ptr);
        CHECK_EQ(ecs.to_expr(position, ptr).c_str(), doctest::String("{x: 10, y: 20}"));

        ptr = e.get(rotation);
        REQUIRE(ptr);
        CHECK_EQ(ecs.to_expr(rotation, ptr).c_str(), doctest::String("{deg: 45}"));
    }

    SUBCASE("record changes to system") {
        auto e = ecs.entity("AnEntity");
        e.set<Position>({10.0, 20.0});

        auto *pPos = e.get<Position>();
        REQUIRE(pPos);
        CHECK_EQ(pPos->x, doctest::Approx(10.0));
        CHECK_EQ(pPos->y, doctest::Approx(20.0));

        struct ObservedEventData {
            flecs::entity id;
            std::string value;
        };

        struct ObservedEvent {
            flecs::entity evt;
            flecs::id id;
            std::vector< ObservedEventData > data;
        };

        std::deque<ObservedEvent> evts;

        auto observer = ecs.observer()
                //.term(flecs::Any)
                .term(flecs::Wildcard)
                .event(flecs::OnAdd)
                .event(flecs::OnRemove)
                .event(flecs::OnSet)
                .iter([&evts](flecs::iter& it) {
                    //debug_log_evt(it);
                    ObservedEvent evt;
                    evt.evt = it.event();
                    evt.id = it.event_id();

                    auto compId = it.event_id();

                    std::cout << " - " << it.event().name() << ": "
                              << it.event_id().str() << ":\n";

                    for (auto i : it) {
                        std::string value;
                        auto* ptr = it.event() != flecs::OnAdd ? it.entity(i).get(compId) : nullptr;
                        if (ptr) {
                            auto expr = it.world().to_expr(compId, ptr);
                            value = expr.c_str();
                        }

                        evt.data.push_back({it.entity(i), value});

                        if (DEBUG_LOGGING) {
                            std::cout << "\t" << it.entity(i).name();
                            if (!value.empty()) {
                                std::cout << "=" << value;
                            }
                            std::cout << std::endl;
                        }

                        evts.push_back(evt);
                    }
                });

        REQUIRE_EQ(evts.size(), 0);

        e.set<Position>({15.0, 42.0});

        REQUIRE_EQ(evts.size(), 1);

        CHECK_EQ(evts.front().evt, flecs::OnSet);
        CHECK_EQ(evts.front().id, position);
        REQUIRE_EQ(evts.front().data.size(), 1);
        CHECK_EQ(evts.front().data[0].id, e);
        CHECK_EQ(evts.front().data[0].value, "{x: 15, y: 42}");
        evts.pop_front();

        e.set<Position>({20.0, 15.0})
         .set<Rotation>({45.0});

        REQUIRE_EQ(evts.size(), 3);

        CHECK_EQ(evts.front().evt, flecs::OnSet);
        CHECK_EQ(evts.front().id, position);
        REQUIRE_EQ(evts.front().data.size(), 1);
        CHECK_EQ(evts.front().data[0].id, e);
        CHECK_EQ(evts.front().data[0].value, "{x: 20, y: 15}");
        evts.pop_front();

        CHECK_EQ(evts.front().evt, flecs::OnAdd);
        CHECK_EQ(evts.front().id, rotation);
        REQUIRE_EQ(evts.front().data.size(), 1);
        CHECK_EQ(evts.front().data[0].id, e);
        CHECK_EQ(evts.front().data[0].value, std::string{});
        evts.pop_front();

        CHECK_EQ(evts.front().evt, flecs::OnSet);
        CHECK_EQ(evts.front().id, rotation);
        REQUIRE_EQ(evts.front().data.size(), 1);
        CHECK_EQ(evts.front().data[0].id, e);
        CHECK_EQ(evts.front().data[0].value, "{deg: 45}");
        evts.pop_front();

        observer.disable();
        observer.destruct();
    }

    SUBCASE("merge changes into system") {
        // TODO: research how to do this - maybe ecs_async_stage_new followed by ecs_merge
        //  see https://www.flecs.dev/flecs/group__commands.html#gaa7dbf97185ee60c86284b7c73e151fd5
    }
}
