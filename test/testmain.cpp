#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "actor.hpp"

TEST_CASE("Default Actor is an '@'", "[actor]") {
    Actor a{};
    REQUIRE(a.ch == '@');
}
