#include <catch2/catch_test_macros.hpp>
#include "frost/core/result.hpp"

using namespace frost;

TEST_CASE("Result success", "[result]") {
    SECTION("value result") {
        Result<int> result = 42;

        REQUIRE(result.is_ok());
        REQUIRE_FALSE(result.is_err());
        REQUIRE(result.value() == 42);
    }

    SECTION("void result") {
        Result<void> result = {};

        REQUIRE(result.is_ok());
        REQUIRE_FALSE(result.is_err());
    }

    SECTION("implicit bool conversion") {
        Result<int> success = 42;
        Result<int> error = Error{ErrorCode::Unknown, "fail"};

        REQUIRE(success);
        REQUIRE_FALSE(error);
    }
}

TEST_CASE("Result error", "[result]") {
    SECTION("error with message") {
        Result<int> result = Error{ErrorCode::NotFound, "Something went wrong"};

        REQUIRE_FALSE(result.is_ok());
        REQUIRE(result.is_err());
        REQUIRE(result.error().message == "Something went wrong");
    }

    SECTION("error with code") {
        Result<int> result = Error{ErrorCode::NotFound, "Not found"};

        REQUIRE(result.error().message == "Not found");
        REQUIRE(result.error().code == ErrorCode::NotFound);
    }

    SECTION("error_code accessor") {
        Result<int> success = 42;
        Result<int> failure = Error{ErrorCode::InvalidArgument};

        REQUIRE(success.error_code() == ErrorCode::None);
        REQUIRE(failure.error_code() == ErrorCode::InvalidArgument);
    }
}

TEST_CASE("Result value_or", "[result]") {
    SECTION("returns value when present") {
        Result<int> result = 42;
        REQUIRE(result.value_or(0) == 42);
    }

    SECTION("returns default when error") {
        Result<int> result = Error{ErrorCode::Unknown, "fail"};
        REQUIRE(result.value_or(0) == 0);
    }
}

TEST_CASE("Result with unique_ptr", "[result]") {
    SECTION("success with unique_ptr") {
        auto ptr = make_unique<int>(42);
        Result<Unique<int>> result = std::move(ptr);

        REQUIRE(result.is_ok());
        REQUIRE(*result.value() == 42);
    }

    SECTION("move value out") {
        Result<Unique<int>> result = make_unique<int>(42);

        auto ptr = std::move(result.value());
        REQUIRE(*ptr == 42);
    }
}

TEST_CASE("Result transformations", "[result]") {
    SECTION("map success") {
        Result<int> result = 10;
        auto mapped = result.map([](int x) { return x * 2; });

        REQUIRE(mapped.is_ok());
        REQUIRE(mapped.value() == 20);
    }

    SECTION("map error propagates") {
        Result<int> result = Error{ErrorCode::NotFound};
        auto mapped = result.map([](int x) { return x * 2; });

        REQUIRE(mapped.is_err());
        REQUIRE(mapped.error_code() == ErrorCode::NotFound);
    }
}
