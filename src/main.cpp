#include "game.cpp"

int main()
{
    Game game;
    if (game.Construct(640, 480, 2, 2))
        game.Start();
    return 0;
}
