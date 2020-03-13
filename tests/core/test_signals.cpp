#include <catch2/catch_test_macros.hpp>
#include "frost/core/signals.hpp"

using namespace frost;

TEST_CASE("Signal basic operations", "[signals]") {
    SECTION("emit to single slot") {
        Signal<int> signal;
        int received = 0;

        signal.connect([&received](int val) {
            received = val;
        });

        signal.emit(42);
        REQUIRE(received == 42);
    }

    SECTION("emit to multiple slots") {
        Signal<int> signal;
        int sum = 0;

        signal.connect([&sum](int val) { sum += val; });
        signal.connect([&sum](int val) { sum += val * 2; });

        signal.emit(10);
        REQUIRE(sum == 30);  // 10 + 20
    }

    SECTION("no args signal") {
        Signal<> signal;
        int call_count = 0;

        signal.connect([&call_count]() {
            call_count++;
        });

        signal.emit();
        signal.emit();
        REQUIRE(call_count == 2);
    }

    SECTION("multiple args signal") {
        Signal<int, float, const char*> signal;
        int i_val = 0;
        float f_val = 0.0f;
        const char* s_val = nullptr;

        signal.connect([&](int i, float f, const char* s) {
            i_val = i;
            f_val = f;
            s_val = s;
        });

        signal.emit(42, 3.14f, "hello");
        REQUIRE(i_val == 42);
        REQUIRE(f_val == 3.14f);
        REQUIRE(std::string(s_val) == "hello");
    }
}

TEST_CASE("Connection management", "[signals]") {
    SECTION("disconnect") {
        Signal<int> signal;
        int received = 0;

        auto conn = signal.connect([&received](int val) {
            received = val;
        });

        signal.emit(1);
        REQUIRE(received == 1);

        conn.disconnect();
        signal.emit(2);
        REQUIRE(received == 1);  // Not updated
    }

    SECTION("disconnect_all") {
        Signal<> signal;
        int count = 0;

        signal.connect([&count]() { count++; });
        signal.connect([&count]() { count++; });
        signal.connect([&count]() { count++; });

        signal.emit();
        REQUIRE(count == 3);

        signal.disconnect_all();
        signal.emit();
        REQUIRE(count == 3);  // No more calls
    }

    SECTION("connection_count") {
        Signal<> signal;

        REQUIRE(signal.connection_count() == 0);
        REQUIRE_FALSE(signal.has_connections());

        auto c1 = signal.connect([]() {});
        REQUIRE(signal.connection_count() == 1);
        REQUIRE(signal.has_connections());

        auto c2 = signal.connect([]() {});
        REQUIRE(signal.connection_count() == 2);

        c1.disconnect();
        signal.emit();  // Trigger cleanup
        REQUIRE(signal.connection_count() == 1);
    }
}

TEST_CASE("ScopedConnection", "[signals]") {
    SECTION("auto disconnect on destruction") {
        Signal<> signal;
        int count = 0;

        {
            ScopedConnection scoped = signal.connect_scoped([&count]() {
                count++;
            });

            signal.emit();
            REQUIRE(count == 1);
        }

        signal.emit();
        REQUIRE(count == 1);  // Not incremented after scope exit
    }

    SECTION("move semantics") {
        Signal<> signal;
        int count = 0;

        ScopedConnection scoped1 = signal.connect_scoped([&count]() {
            count++;
        });

        ScopedConnection scoped2 = std::move(scoped1);

        signal.emit();
        REQUIRE(count == 1);

        scoped2.disconnect();
        signal.emit();
        REQUIRE(count == 1);
    }
}

TEST_CASE("Property with change notification", "[signals][property]") {
    SECTION("value get/set") {
        Property<int> prop(10);

        REQUIRE(prop.get() == 10);
        REQUIRE(*prop == 10);

        prop.set(20);
        REQUIRE(prop.get() == 20);
    }

    SECTION("on_change notification") {
        Property<int> prop(0);
        int old_val = -1;
        int new_val = -1;

        prop.on_change([&](const int& old_v, const int& new_v) {
            old_val = old_v;
            new_val = new_v;
        });

        prop.set(42);
        REQUIRE(old_val == 0);
        REQUIRE(new_val == 42);
    }

    SECTION("no notification when value unchanged") {
        Property<int> prop(10);
        int change_count = 0;

        prop.on_change([&](const int&, const int&) {
            change_count++;
        });

        prop.set(10);  // Same value
        REQUIRE(change_count == 0);

        prop.set(20);  // Different value
        REQUIRE(change_count == 1);
    }
}
