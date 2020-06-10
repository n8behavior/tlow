#include "game.cpp"

int main()
{
    constexpr int base_res {32};
    constexpr int base_px {2};
    Game game;
    if (game.Construct(base_res*16, base_res*9, base_px, base_px, false, true)) {
        game.Start();
    }
    return 0;
}

