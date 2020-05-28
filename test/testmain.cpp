#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "game.cpp"

TEST_CASE("Title is correct", "[game]") {
    Game g{};
    REQUIRE(g.sAppName == "The Lie of Winterhaven");
}
