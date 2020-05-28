#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.hpp"

// Override base class with your custom functionality
class Game : public olc::PixelGameEngine {
  public:
    Game() {
        // Name you application
        sAppName = "The Lie of Winterhaven";
    }

    float x = 100.0f;
    float y = 100.0f;

    bool OnUserCreate() override {
        // Called once at the start, so create things here
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override {
        if (GetKey(olc::Key::UP).bHeld) y = y - 30.0f * fElapsedTime;
        if (GetKey(olc::Key::DOWN).bHeld) y = y + 30.0f * fElapsedTime;
        if (GetKey(olc::Key::LEFT).bHeld) x = x - 30.0f * fElapsedTime;
        if (GetKey(olc::Key::RIGHT).bHeld) x = x + 30.0f * fElapsedTime;


        Clear(olc::BLACK);
        FillRect(x, y, 10, 10, olc::YELLOW);

        return true;
    }
};
