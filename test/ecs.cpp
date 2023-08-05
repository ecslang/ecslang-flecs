#include <iostream>
#include <deque>
#include <vector>

#include "flecs.h"
#include "doctest.h"
#include "defer.h"

struct Vec2d {
    double x, y;
};

struct Position {
    double x {};
    double y {};
};

struct Rotation {
    double deg;
};

struct AddPosition {
    double x {};
    double y {};
};

#define CHECK_COMPONENT_EQ(e, CompType, jsonStr) \
    CHECK_EQ(std::string((e).world().to_expr((e).get<CompType>()).c_str()), std::string(jsonStr))

bool DEBUG_LOGGING = true;
bool OBSERVER_LOGGING = false;
bool TABLE_LOGGING = false;

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

    auto position = ecs.component<Position>()
            .member<double>("x")
            .member<double>("y");

    auto rotation = ecs.component<Rotation>()
            .member<double>("deg");

    auto addPosition = ecs.component<AddPosition>()
            .member<double>("x")
            .member<double>("y");

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
        CHECK_COMPONENT_EQ(e, Position, "{x: 10, y: 20}");

        ptr = e.get(rotation);
        REQUIRE(ptr);
        CHECK_EQ(ecs.to_expr(rotation, ptr).c_str(), doctest::String("{deg: 45}"));
        CHECK_COMPONENT_EQ(e, Rotation, "{deg: 45}");
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

                if (OBSERVER_LOGGING) {
                    std::cout << " - " << it.event().name() << ": "
                              << it.event_id().str() << ":\n";
                }

                for (auto i : it) {
                    std::string value;
                    auto* ptr = it.event() != flecs::OnAdd ? it.entity(i).get(compId) : nullptr;
                    if (ptr) {
                        auto expr = it.world().to_expr(compId, ptr);
                        value = expr.c_str();
                    }

                    evt.data.push_back({it.entity(i), value});

                    if (OBSERVER_LOGGING) {
                        std::cout << "\t" << it.entity(i).path().c_str();
                        if (!value.empty()) {
                            std::cout << "=" << value;
                        }
                        std::cout << std::endl;
                    }
                }

                evts.push_back(evt);
            });

        Defer defer{[&] {
            observer.disable();
            observer.destruct();
        }};

        SUBCASE("single component changes") {
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
        }

        SUBCASE("multiple component changes") {
            e.set<AddPosition>({-10.0, -20.0});
            auto e1 = ecs.entity().add<Position>().set<AddPosition>({2, 1});
            auto e2 = ecs.entity().add<Position>().set<AddPosition>({4, 2});
            auto e3 = ecs.entity().add<Position>().set<AddPosition>({6, 3});

            auto q_write = ecs.query<Position, const AddPosition>();
            auto q_read = ecs.query_builder<const Position>()
                .build();

            CHECK(q_read.changed());
            q_read.iter([](flecs::iter&) {
            });
            CHECK_FALSE(q_read.changed());

            auto o = ecs.observer<Position>()
                .event(flecs::OnSet)
                .iter([](flecs::iter& it) {
                    std::cout << "[Position:OnSet] " << it.event().name() << ": "
                              << it.event_id().str() << ":\n";
                });
            Defer destructO{[&] {
                o.disable();
                o.destruct();
            }};

            evts.clear();

            CHECK_COMPONENT_EQ(e, Position, "{x: 10, y: 20}");
            CHECK_COMPONENT_EQ(e1, Position, "{x: 0, y: 0}");

            q_write.each([](Position& pos, const AddPosition& add) {
                pos.x += add.x;
                pos.y += add.y;
            });

            CHECK_COMPONENT_EQ(e, Position, "{x: 0, y: 0}");
            CHECK_COMPONENT_EQ(e1, Position, "{x: 2, y: 1}");

            REQUIRE_EQ(evts.size(), 0);
            CHECK(q_read.changed());

            SUBCASE("write to component triggers OnSet event") {
                SUBCASE("set") {
                    e1.set<Position>({4, 2});
                    REQUIRE_EQ(evts.size(), 1);
                    CHECK(q_read.changed());
                }
                SUBCASE("get_mut + modified") {
                    *e1.get_mut<Position>() = {4, 2};
                    e1.modified<Position>();
                }

                CHECK_COMPONENT_EQ(e1, Position, "{x: 4, y: 2}");

                CHECK(q_read.changed());
                REQUIRE_EQ(evts.size(), 1);
                CHECK_EQ(evts.front().evt, flecs::OnSet);
                CHECK_EQ(evts.front().id, position);
                REQUIRE_EQ(evts.front().data.size(), 1);
                CHECK_EQ(evts.front().data[0].id, e1);
                CHECK_EQ(evts.front().data[0].value, "{x: 4, y: 2}");
                evts.pop_front();
            }

            q_read.iter([](flecs::iter&) {
            });
            CHECK_FALSE(q_read.changed());

            SUBCASE("defer write") {
                CHECK_FALSE(q_read.changed());

                CHECK_COMPONENT_EQ(e1, Position, "{x: 2, y: 1}");

                CHECK_FALSE(ecs.is_readonly());
                ecs.readonly_begin();
                CHECK(ecs.is_readonly());

                SUBCASE("set") {
                    e1.set<Position>({4, 2});
                }
                SUBCASE("get_mut + modified") {
                    *e1.get_mut<Position>() = {4, 2};
                    e1.modified<Position>();
                }

                CHECK(evts.empty());
                CHECK_FALSE(q_read.changed());

                CHECK(ecs.is_readonly());
                ecs.readonly_end();
                CHECK_FALSE(ecs.is_readonly());

                CHECK(q_read.changed());
                CHECK_COMPONENT_EQ(e1, Position, "{x: 4, y: 2}");

                REQUIRE_EQ(evts.size(), 1);
                CHECK_EQ(evts.front().evt, flecs::OnSet);
                CHECK_EQ(evts.front().id, position);
                REQUIRE_EQ(evts.front().data.size(), 1);
                CHECK_EQ(evts.front().data[0].id, e1);
                CHECK_EQ(evts.front().data[0].value, "{x: 4, y: 2}");
                evts.pop_front();
            }
        }
    }

    SUBCASE("query and sync component changes") {
        auto e1 = ecs.entity().add<Position>().set<AddPosition>({2, 1});
        auto e2 = ecs.entity().add<Position>().set<AddPosition>({4, 2});
        auto e3 = ecs.entity().add<Position>().set<AddPosition>({6, 3});
        auto e4 = ecs.entity().set<Position>({-1, -2}).set<AddPosition>({0, 0});
        auto e5 = ecs.entity().set<Position>({-5, -10});

        auto sMove = ecs.system<Position, const AddPosition>()
            .each([](Position& pos, const AddPosition& add) {
                pos.x += add.x;
                pos.y += add.y;
            });

        auto qRead = ecs.query_builder<const Position>()
            .instanced()
            .build();

        qRead.changed();
        auto syncChanges = [&](const char* title) -> size_t {
            size_t active = 0;

            if (TABLE_LOGGING) {
                std::cout << "------------------------------------------------------" << std::endl;
                std::cout << title << std::endl;
                std::cout << "------------------------------------------------------" << std::endl;
            }
            qRead.iter([&active](flecs::iter &it, const Position *pos) {
                if (TABLE_LOGGING) {
                    std::cout << "TABLE (entries: " << it.count() << ", changed: " << it.changed() << ")  {\n";
                }
                if (it.changed()) {
                    active += it.count();
                    for (auto i: it) {
                        if (TABLE_LOGGING) {
                            std::cout << " - " << it.entity(i).path() << ": Position={" << pos[i].x << ", " << pos[i].y
                                      << "}\n";
                        }
                    }
                }
                else {
                    it.skip();
                }
                if (TABLE_LOGGING) {
                    std::cout << "}\n";
                }
            });

            return active;
        };

        sMove.run();

        CHECK_EQ(syncChanges("Initial sync"), 5);

        CHECK_EQ(syncChanges("Empty sync"), 0);

        sMove.run();
        CHECK_EQ(syncChanges("Partial sync #1"), 4);

        e2.disable<AddPosition>();
        e4.disable<AddPosition>();

        sMove.run();
        CHECK_EQ(syncChanges("Partial sync #2"), 4);

        sMove.run();
        CHECK_EQ(syncChanges("Partial sync #3"), 2);
    }
}
