#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.hpp"

// Override base class with your custom functionality
class Game : public olc::PixelGameEngine {
public:
    Game() {
        // Name you application
        sAppName = "The Lie of Winterhaven";
    }

    struct Renderable {
        Renderable() {}

        void load(const std::string& file) {
            sprite = new olc::Sprite(file);
            decal = new olc::Decal(sprite);
        }

        ~Renderable() {
            delete decal;
            delete sprite;
        }
        olc::Sprite* sprite = nullptr;
        olc::Decal* decal = nullptr;
    };

    struct vec3d {
        float x, y, z;
    };

    struct Quad {
        vec3d points[4];
        olc::vf2d tile;
    };

    struct Cell {
        bool wall = false;
        olc::vi2d id[6]{ };
    };

    class World {
    public:
        World() {

        }

        void create(int w, int h) {
            size = { w, h };
            cells.resize(w * h);
        }

        Cell& getCell(const olc::vi2d& v) {
            if (v.x >= 0 && v.x < size.x && v.y >=0 && v.y < size.y)
                return cells[v.y * size.x + v.x];
            else
                return nullCell;
        }
        olc::vi2d size;

    private:
        std::vector<Cell> cells;
        Cell nullCell;
    };

    World world;
    Renderable rendAllWalls;

    olc::vf2d  cameraPos = { 0.0f, 0.0f };
    float cameraAngle = 0.0f;
    float cameraPitch = 5.5f;
    float cameraZoom = 16.0f;

    olc::vi2d tileSize = { 32, 32};

    enum Face {
        Floor = 0,
        North = 1,
        East = 2,
        South = 3,
        West = 4,
        Top = 5
    };

    bool OnUserCreate() override {
        rendAllWalls.load("./sprites.png");

        world.create(64, 64);

        for (int y = 0; y < world.size.y; y++) {
            for (int x = 0; x < world.size.x; x++) {
                world.getCell({ x, y }).wall = false;
                world.getCell({ x, y }).id[Face::Floor] = olc::vi2d{ 3, 1 } * tileSize;
                world.getCell({ x, y }).id[Face::Top] = olc::vi2d{ 21, 8 } * tileSize;
                world.getCell({ x, y }).id[Face::North] = olc::vi2d{ 1, 6 } * tileSize;
                world.getCell({ x, y }).id[Face::South] = olc::vi2d{ 1, 6 } * tileSize;
                world.getCell({ x, y }).id[Face::West] = olc::vi2d{ 1, 6 } * tileSize;
                world.getCell({ x, y }).id[Face::East] = olc::vi2d{ 1, 6 } * tileSize;
            }
        }


        return true;
    }

    std::array<vec3d, 8> createCube(const olc::vi2d& cell, const float angle, const float pitch, const float scale, const vec3d& camera) {
        // Unit Cube
        std:: array<vec3d, 8> unitCube, rotCube, worldCube, projCube;

        unitCube[0] = { 0.0f, 0.0f, 0.0f };
        unitCube[1] = { scale, 0.0f, 0.0f };
        unitCube[2] = { scale, -scale, 0.0f };
        unitCube[3] = { 0.0f, -scale, 0.0f };
        unitCube[4] = { 0.0f, 0.0f, scale, };
        unitCube[5] = { scale, 0.0f, scale, };
        unitCube[6] = { scale, -scale, scale, };
        unitCube[7] = { 0.0f, -scale, scale, };

        // Translate Cube in X-Z Plane
        for (int i = 0; i < 8; i++) {
            unitCube[i].x += (cell.x * scale - camera.x);
            unitCube[i].y += -camera.y;
            unitCube[i].z += (cell.y * scale - camera.z);
        }

        //Rotate Cube in Y-Axis around origin
        float s = sin(angle);
        float c = cos(angle);
        for (int i = 0; i < 8; i++) {
            rotCube[i].x = unitCube[i].x * c + unitCube[i].z * s;
            rotCube[i].y = unitCube[i].y;
            rotCube[i].z = unitCube[i].x * -s + unitCube[i].z * c;
        }

        //Rotate Cube in X-Axis around origin (tilt slightly overhead)
        s = sin(pitch);
        c = cos(pitch);
        for (int i = 0; i < 8; i++) {
            worldCube[i].x = rotCube[i].x;
            worldCube[i].y = rotCube[i].y * c - rotCube[i].z * s;
            worldCube[i].z = rotCube[i].y * s + rotCube[i].z * c;
        }

        // Project Cube Orthographically - Full Screen Centered
        for (int i = 0; i < 8; i++) {
            projCube[i].x = worldCube[i].x + ScreenWidth() * 0.5f;
            projCube[i].y = worldCube[i].y + ScreenHeight() * 0.5f;
            projCube[i].z = worldCube[i].z;
        }

        return projCube;
    }

    bool OnUserUpdate(float fElapsedTime) override {
        std::vector<Quad> quads;

        for (int y = 0; y < world.size.y; y++)
            for (int x = 0; x < world.size.x; x++)
                getFaceQuads({x, y}, cameraAngle, cameraPitch, cameraZoom, {cameraPos.x, cameraPos.y}, quads);

        Clear(olc::VERY_DARK_GREY);
        for (auto& q : quads)
            DrawPartialWarpedDecal (
                    rendAllWalls.decal,
                    {
                            { q.points[0].x, q.points[0].y },
                            { q.points[1].x, q.points[1].y },
                            { q.points[2].x, q.points[2].y },
                            { q.points[3].x, q.points[3].y },
                    },
                    q.tile,
                    tileSize );

        return true;
    }

    void getFaceQuads(const olc::vi2d& cell, const float cameraAngle, const float cameraPitch, const float scale, const vec3d& camera, std::vector<Quad>& render) {
        std::array<vec3d, 8> projCube = createCube(cell, cameraAngle, cameraPitch, scale, camera);
        auto& c = world.getCell(cell);

        auto makeFace = [&](int v1, int v2, int v3, int v4, Face f) {
            render.push_back({ projCube[v1], projCube[v2], projCube[v3], projCube[v4], c.id[f]});
        };

        if (!c.wall) {
            makeFace(4, 0, 1, 5, Face::Floor);
        } else {
            makeFace(3, 0, 1, 2, Face::South);
            makeFace(6, 5, 4, 7, Face::North);
            makeFace(7, 4, 0, 3, Face::East);
            makeFace(2, 1, 5, 6, Face::West);
            makeFace(7, 3, 2, 6, Face::Top);
        }

    }
};
