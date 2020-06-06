#include "game.cpp"

int main()
{
    Game game;
    if (game.Construct(16*32, 9 * 32, 2, 2))
        game.Start();
    return 0;
}
