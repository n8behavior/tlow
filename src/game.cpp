#define OLC_PGE_APPLICATION
#define OLC_PGEX_POPUPMENU

#include "olcPixelGameEngine.hpp"
#include "olcPGEX_PopUpMenu.hpp"

#include <fstream>
#include <filesystem>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
//#include <execution>

class Game : public olc::PixelGameEngine {
public:
    Game() {
        sAppName = "The Lie of Winterhaven";
    }

    struct Renderable
    {
        Renderable() {}

        void Load(const std::string& sFile)
        {
            sprite = new olc::Sprite(sFile);
            decal = new olc::Decal(sprite);
        }

        ~Renderable()
        {
            delete decal;
            delete sprite;
        }

        olc::Sprite* sprite = nullptr;
        olc::Decal* decal = nullptr;
    };

    struct vec3d
    {
        float x, y, z;
    };

    enum Face
    {
        Floor = 0,
        North = 1,
        East = 2,
        South = 3,
        West = 4,
        Top = 5
    };

    std::array<std::string,6> face_names {"Floor", "North", "East", "South", "West", "Top" };

    struct sQuad
    {
        vec3d points[4];
        olc::vf2d tile;
        olc::vf2d cell;
        Face face;
    };

    struct sCell
    {
        bool wall = false;
        olc::vi2d id[6]{  };

        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & wall;
            ar & id;
        }
    };

    class World
    {
        friend class boost::serialization::access;

    public:
        World() = default;

        void Create(int w, int h)
        {
            size = { w, h };
            vCells.resize(w * h);
        }

        sCell& GetCell(const olc::vi2d& v)
        {
            if (v.x >= 0 && v.x < size.x && v.y >= 0 && v.y < size.y)
                return vCells[v.y * size.x + v.x];
            else
                return NullCell;
        }

    public:
        olc::vi2d size;

    private:
        std::vector<sCell> vCells;
        sCell NullCell;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & vCells;
            ar & size;
        }

    };


    World world;
    bool saving = false;
    std::string map_name { "test.map" };

    Renderable rendSelect;
    Renderable rendAllWalls;
    Renderable menu_sprites;

    olc::vf2d vCameraPos = { 0.0f, 0.0f };
    float fCameraAngle = 3.6f;
    float fCameraAngleTarget = fCameraAngle;
    float fCameraPitch = 5.8f;
    float fCameraZoom = 16.0f;

    bool bVisible[6]; // if any part of the quad is visible

    Face mouse_face {Face::Floor};
    olc::vi2d mouse_cell {0, 0};
    olc::vi2d vCursor = { 16, 16 };
    olc::vi2d vTileCursor = { 0,0 };
    olc::vi2d vTileSize = { 32, 32 };

    olc::popup::Menu m;
    olc::popup::Manager man;
    uint8_t menu_layer;
    std::string sLastAction;
    bool menu_showing{false};
    enum menu_commands {
        save = 0,
        save_as = 1
    };

    uint8_t editor_layer;

public:
    bool OnUserCreate() override
    {
        //SetDrawTarget((uint8_t)0);
        Clear(olc::BLANK);

        menu_layer = CreateLayer();
        EnableLayer(menu_layer, true);
        SetDrawTarget(menu_layer);
        Clear(olc::BLANK);

        editor_layer = CreateLayer();
        EnableLayer(editor_layer, true);

        rendSelect.Load("./cursor.png");
        rendAllWalls.Load("./sprites.png");
        menu_sprites.Load("./menu.png");

        if (std::filesystem::exists(map_name)) {

            std::cout << "Loading map from " << map_name << std::endl;
            std::ifstream ifs(map_name);
            boost::archive::text_iarchive ia(ifs);
            ia >> world;
        } else {
            // Default 32x32 world
            world.Create(16, 16);

            for (int y = 0; y < world.size.y; y++)
                for (int x = 0; x < world.size.x; x++) {
                    world.GetCell({x, y}).wall = false;
                    world.GetCell({x, y}).id[Face::Floor] = olc::vi2d{10, 10} * vTileSize;
                    world.GetCell({x, y}).id[Face::Top] = olc::vi2d{14, 10} * vTileSize;
                    world.GetCell({x, y}).id[Face::North] = olc::vi2d{6, 1} * vTileSize;
                    world.GetCell({x, y}).id[Face::South] = olc::vi2d{6, 1} * vTileSize;
                    world.GetCell({x, y}).id[Face::West] = olc::vi2d{6, 1} * vTileSize;
                    world.GetCell({x, y}).id[Face::East] = olc::vi2d{6, 1} * vTileSize;
                }
        }
        // Construction (root menu is a 1x5 table)
        m.SetTable(1, 5);
        m["Save"].SetID(menu_commands::save);
        m["Save As"].SetID(menu_commands::save_as);
        // Menu DEMO code for reference
        // Add first item  to root menu (A 1x5 submenu)
        m["Menu1"].SetTable(1, 5);

        // Add items to first item
        m["Menu1"]["Item1"];
        m["Menu1"]["Item2"];

        // Add a 4x3 submenu
        m["Menu1"]["Item3"].SetTable(4, 3);
        m["Menu1"]["Item3"]["Option1"];
        m["Menu1"]["Item3"]["Option2"];

        // Set properties of specific item
        m["Menu1"]["Item3"]["Option3"].Enable(false);
        m["Menu1"]["Item3"]["Option4"];
        m["Menu1"]["Item3"]["Option5"];
        m["Menu1"]["Item4"];

        // Add second item to root menu
        m["Menu2"].SetTable(3, 3);
        m["Menu2"]["Item1"];
        m["Menu2"]["Item2"].SetID(1001).Enable(true);
        m["Menu2"]["Item3"];

        // Construct the menu structure
        m.Build();


        return true;
    }

    // Given the camera and upper left corner of the cell floor, project the other 7 vertices
    std::array<vec3d, 8> CreateCube(const olc::vi2d& vCell, const float fAngle, const float fPitch, const float fScale, const vec3d& vCamera)
    {
        // Unit Cube
        std::array<vec3d, 8> unitCube, rotCube, worldCube, projCube;
        unitCube[0] = { 0.0f, 0.0f, 0.0f };
        unitCube[1] = { fScale, 0.0f, 0.0f };
        unitCube[2] = { fScale, -fScale, 0.0f };
        unitCube[3] = { 0.0f, -fScale, 0.0f };
        unitCube[4] = { 0.0f, 0.0f, fScale };
        unitCube[5] = { fScale, 0.0f, fScale };
        unitCube[6] = { fScale, -fScale, fScale };
        unitCube[7] = { 0.0f, -fScale, fScale };

        // Translate Cube in X-Z Plane
        for (int i = 0; i < 8; i++)
        {
            unitCube[i].x += (vCell.x * fScale - vCamera.x);
            unitCube[i].y += -vCamera.y;
            unitCube[i].z += (vCell.y * fScale - vCamera.z);
        }

        // Rotate Cube in Y-Axis around origin
        float s = sin(fAngle);
        float c = cos(fAngle);
        for (int i = 0; i < 8; i++)
        {
            rotCube[i].x = unitCube[i].x * c + unitCube[i].z * s;
            rotCube[i].y = unitCube[i].y;
            rotCube[i].z = unitCube[i].x * -s + unitCube[i].z * c;
        }

        // Rotate Cube in X-Axis around origin (tilt slighly overhead)
        s = sin(fPitch);
        c = cos(fPitch);
        for (int i = 0; i < 8; i++)
        {
            worldCube[i].x = rotCube[i].x;
            worldCube[i].y = rotCube[i].y * c - rotCube[i].z * s;
            worldCube[i].z = rotCube[i].y * s + rotCube[i].z * c;
        }

        /* Project Cube Orthographically - Unit Cube Viewport
        float fLeft = -ScreenWidth() * 0.5f;
        float fRight = ScreenWidth() * 0.5f;
        float fTop = ScreenHeight() * 0.5f;
        float fBottom = -ScreenHeight() * 0.5f;
        float fNear = 0.1f;
        float fFar = 100.0f;
        for (int i = 0; i < 8; i++)
        {
        	projCube[i].x = (2.0f / (fRight - fLeft)) * worldCube[i].x - ((fRight + fLeft) / (fRight - fLeft));
        	projCube[i].y = (2.0f / (fTop - fBottom)) * worldCube[i].y - ((fTop + fBottom) / (fTop - fBottom));
        	projCube[i].z = (2.0f / (fFar - fNear)) * worldCube[i].z - ((fFar + fNear) / (fFar - fNear));
          projCube[i].x *= -fRight;
          projCube[i].y *= -fTop;
          projCube[i].x += fRight;
          projCube[i].y += fTop;
        }*/

        // Project Cube Orthographically - Full Screen Centered
        for (int i = 0; i < 8; i++)
        {
            projCube[i].x = worldCube[i].x + ScreenWidth() * 0.5f;
            projCube[i].y = worldCube[i].y + ScreenHeight() * 0.5f;
            projCube[i].z = worldCube[i].z;
        }

        return projCube;
    }



    void CalculateVisibleFaces(std::array<vec3d, 8>& cube)
    {
        auto CheckNormal = [&](int v1, int v2, int v3)
        {
            olc::vf2d a = { cube[v1].x, cube[v1].y };
            olc::vf2d b = { cube[v2].x, cube[v2].y };
            olc::vf2d c = { cube[v3].x, cube[v3].y };
            return  (b - a).cross(c - a) > 0;
        };

        bVisible[Face::Floor] = CheckNormal(4, 0, 1);
        bVisible[Face::South] = CheckNormal(3, 0, 1);
        bVisible[Face::North] = CheckNormal(6, 5, 4);
        bVisible[Face::East] = CheckNormal(7, 4, 0);
        bVisible[Face::West] = CheckNormal(2, 1, 5);
        bVisible[Face::Top] = CheckNormal(7, 3, 2);
    }

    void GetFaceQuads(const olc::vi2d& vCellFloorVertex1, const float fAngle, const float fPitch, const float fScale, const vec3d& vCamera, std::vector<sQuad> &quads, std::vector<sQuad> &mouse_quads)
    {
        std::array<vec3d, 8> projCube = CreateCube(vCellFloorVertex1, fAngle, fPitch, fScale, vCamera);

        auto& cell = world.GetCell(vCellFloorVertex1);

        auto MakeFace = [&](int v1, int v2, int v3, int v4, Face f)
        {
            // Test is mouse pointer is in this face
            auto x = GetMouseX();
            auto y = GetMouseY();
            auto pv1  = (projCube[v2].y - projCube[v1].y) * (x - projCube[v1].x) + (projCube[v1].x - projCube[v2].x) * (y - projCube[v1].y);
            auto pv2  = (projCube[v3].y - projCube[v2].y) * (x - projCube[v2].x) + (projCube[v2].x - projCube[v3].x) * (y - projCube[v2].y);
            auto pv3  = (projCube[v4].y - projCube[v3].y) * (x - projCube[v3].x) + (projCube[v3].x - projCube[v4].x) * (y - projCube[v3].y);
            auto pv4  = (projCube[v1].y - projCube[v4].y) * (x - projCube[v4].x) + (projCube[v4].x - projCube[v1].x) * (y - projCube[v4].y);
            sQuad q = {projCube[v1], projCube[v2], projCube[v3], projCube[v4], cell.id[f] };
            if (pv1 < 0 && pv2 < 0 && pv3 < 0 && pv4 < 0) {
                q.cell = vCellFloorVertex1;
                q.face = f;
                mouse_quads.push_back(q);
            }
            quads.push_back(q);
        };

        if (!cell.wall)
        {
            if(bVisible[Face::Floor]) MakeFace(4, 0, 1, 5, Face::Floor);
        }
        else
        {
            if (bVisible[Face::South]) MakeFace(3, 0, 1, 2, Face::South);
            if (bVisible[Face::North]) MakeFace(6, 5, 4, 7, Face::North);
            if (bVisible[Face::East]) MakeFace(7, 4, 0, 3, Face::East);
            if (bVisible[Face::West]) MakeFace(2, 1, 5, 6, Face::West);
            if (bVisible[Face::Top]) MakeFace(7, 3, 2, 6, Face::Top);
        }
    }


    void save_map() {
        // TODO: this is a bad idea, needs proper synchronized access
        //       and World (Game too??) needs locked while saving
        if (saving) return;
        saving = true;
        std::cout << "Saving map to " << map_name << std::endl;
        std::ofstream ofs(map_name);
        boost::archive::text_oarchive oa(ofs);
        oa << world;
        std::cout << "Saved " << map_name << std::endl;
        saving = false;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        SetDrawTarget(editor_layer);
        Clear(olc::VERY_DARK_BLUE);

        // Grab mouse for convenience
        olc::vi2d vMouse = { GetMouseX(), GetMouseY() };

        // CTRL modifier is being used
        if (GetKey(olc::Key::CTRL).bHeld)
        {
            // Save the map
            if( GetKey(olc::Key::S).bPressed && !saving)
            {
                save_map();
            }
        } else if (menu_showing) {
            // Send key events to menu
            if (GetKey(olc::Key::UP).bPressed) man.OnUp();
            if (GetKey(olc::Key::DOWN).bPressed) man.OnDown();
            if (GetKey(olc::Key::LEFT).bPressed) man.OnLeft();
            if (GetKey(olc::Key::RIGHT).bPressed) man.OnRight();
            if (GetKey(olc::Key::Z).bPressed) {
                man.Close();
                menu_showing = false;
            }

            // "Confirm/Action" Key does something, if it returns non-null
            // then a menu item has been selected. The specific item will
            // be returned
            olc::popup::Menu* command = nullptr;
            if (GetKey(olc::Key::SPACE).bPressed) command = man.OnConfirm();
            if (command != nullptr)
            {
                sLastAction =
                        "Selected: " + command->GetName() +
                        " ID: " + std::to_string(command->GetID());

                // Optionally close menu?
                man.Close();
                menu_showing = false;
                switch (command->GetID()) {
                    case menu_commands::save:
                        save_map();
                        break;
                }
            }
        } else { // no modifier in use
            if (GetKey(olc::Key::M).bPressed) {
                man.Open(&m);
                menu_showing = true;
                std::cout << "Menu"  << std::endl;

            }



            // Edit mode - Selection from tile sprite sheet
            if (GetKey(olc::Key::TAB).bHeld) {
                DrawSprite({0, 0}, rendAllWalls.sprite);
                DrawRect(vTileCursor * vTileSize, vTileSize);
                if (GetMouse(0).bPressed) vTileCursor = vMouse / vTileSize;
                return true;
            }

            // WS keys to tilt camera
            if (GetKey(olc::Key::W).bHeld) fCameraPitch += 1.0f * fElapsedTime;
            if (GetKey(olc::Key::S).bHeld) fCameraPitch -= 1.0f * fElapsedTime;

            // DA Keys to manually rotate camera
            if (GetKey(olc::Key::D).bHeld) fCameraAngleTarget += 1.0f * fElapsedTime;
            if (GetKey(olc::Key::A).bHeld) fCameraAngleTarget -= 1.0f * fElapsedTime;

            // QZ Keys to zoom in or out
            if (GetKey(olc::Key::Q).bHeld) fCameraZoom += 5.0f * fElapsedTime;
            if (GetKey(olc::Key::Z).bHeld) fCameraZoom -= 5.0f * fElapsedTime;
            // Zoom proportional to mouse wheel delta
            fCameraZoom *= 1 - GetMouseWheel() * 0.001f;

            // Numpad keys used to rotate camera to fixed angles
            if (GetKey(olc::Key::NP2).bPressed) fCameraAngleTarget = 3.14159f * 0.0f;
            if (GetKey(olc::Key::NP1).bPressed) fCameraAngleTarget = 3.14159f * 0.25f;
            if (GetKey(olc::Key::NP4).bPressed) fCameraAngleTarget = 3.14159f * 0.5f;
            if (GetKey(olc::Key::NP7).bPressed) fCameraAngleTarget = 3.14159f * 0.75f;
            if (GetKey(olc::Key::NP8).bPressed) fCameraAngleTarget = 3.14159f * 1.0f;
            if (GetKey(olc::Key::NP9).bPressed) fCameraAngleTarget = 3.14159f * 1.25f;
            if (GetKey(olc::Key::NP6).bPressed) fCameraAngleTarget = 3.14159f * 1.5f;
            if (GetKey(olc::Key::NP3).bPressed) fCameraAngleTarget = 3.14159f * 1.75f;

            // Numeric keys apply selected tile to specific face
            if (GetKey(olc::Key::K1).bPressed) world.GetCell(vCursor).id[Face::North] = vTileCursor * vTileSize;
            if (GetKey(olc::Key::K2).bPressed) world.GetCell(vCursor).id[Face::East] = vTileCursor * vTileSize;
            if (GetKey(olc::Key::K3).bPressed) world.GetCell(vCursor).id[Face::South] = vTileCursor * vTileSize;
            if (GetKey(olc::Key::K4).bPressed) world.GetCell(vCursor).id[Face::West] = vTileCursor * vTileSize;
            if (GetKey(olc::Key::K5).bPressed) world.GetCell(vCursor).id[Face::Floor] = vTileCursor * vTileSize;
            if (GetKey(olc::Key::K6).bPressed) world.GetCell(vCursor).id[Face::Top] = vTileCursor * vTileSize;

            // Smooth camera
            fCameraAngle += (fCameraAngleTarget - fCameraAngle) * 10.0f * fElapsedTime;

            // Arrow keys to move the selection cursor around map (boundary checked)
            if (GetKey(olc::Key::LEFT).bPressed) vCursor.x++;
            if (GetKey(olc::Key::RIGHT).bPressed) vCursor.x--;
            if (GetKey(olc::Key::UP).bPressed) vCursor.y++;
            if (GetKey(olc::Key::DOWN).bPressed) vCursor.y--;

            // Right mouse-click to jump cursor to location
            if (GetMouse(1).bPressed) vCursor = mouse_cell;

            // Left mouse-click to paint mouse face with current tile
            if (GetMouse(0).bHeld) world.GetCell(mouse_cell).id[mouse_face] = vTileCursor * vTileSize;

            // Keep cursor in bounds
            if (vCursor.x < 0) vCursor.x = 0;
            if (vCursor.y < 0) vCursor.y = 0;
            if (vCursor.x >= world.size.x) vCursor.x = world.size.x - 1;
            if (vCursor.y >= world.size.y) vCursor.y = world.size.y - 1;

            // Place block with space
            if (GetKey(olc::Key::SPACE).bPressed) {
                world.GetCell(vCursor).wall = !world.GetCell(vCursor).wall;
            }
        }

        // Position camera in world
        vCameraPos = { (float)vCursor.x, (float)vCursor.y };
        vCameraPos *= fCameraZoom;

        // Rendering

        // 1) Create dummy cube to extract visible face information
        // Cull faces that cannot be seen
        std::array<vec3d, 8> cullCube = CreateCube({ 0, 0 }, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y });
        CalculateVisibleFaces(cullCube);

        // 2) Get all visible sides of all visible "tile cubes"
        std::vector<sQuad> vQuads;
        std::vector<sQuad> mouse_quads; // cube face containing mouse pointer
        for(int y = 0; y<world.size.y; y++)
            for(int x=0; x<world.size.x; x++)
                GetFaceQuads({ x, y }, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y }, vQuads, mouse_quads);

        // 3) Sort in order of depth, from farthest away to closest
        std::sort(/*std::execution::par_unseq, */vQuads.begin(), vQuads.end(), [](const sQuad& q1, const sQuad& q2)
        {
            float z1 = (q1.points[0].z + q1.points[1].z + q1.points[2].z + q1.points[3].z) * 0.25f;
            float z2 = (q2.points[0].z + q2.points[1].z + q2.points[2].z + q2.points[3].z) * 0.25f;
            return z1 < z2;
        });

        // 4) Iterate through all "tile cubes" and draw their visible faces
        for (auto& q : vQuads)
            DrawPartialWarpedDecal
                    (
                            rendAllWalls.decal,
                            { {q.points[0].x, q.points[0].y}, {q.points[1].x, q.points[1].y}, {q.points[2].x, q.points[2].y}, {q.points[3].x, q.points[3].y} },
                            q.tile,
                            vTileSize
                    );

        // 5) Draw current tile selection
        DrawPartialDecal({ (float)(ScreenWidth() - vTileSize.x - 10), 10 }, rendAllWalls.decal, vTileCursor * vTileSize, vTileSize);

        // 6) Draw selection "tile cube"
        vQuads.clear();
        GetFaceQuads(vCursor, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y }, vQuads, mouse_quads);
        for (auto& q : vQuads)
            DrawWarpedDecal(rendSelect.decal, { {q.points[0].x, q.points[0].y}, {q.points[1].x, q.points[1].y}, {q.points[2].x, q.points[2].y}, {q.points[3].x, q.points[3].y} });

        // Sort by depth to closest quad
        std::sort(/*std::execution::par_unseq, */mouse_quads.begin(), mouse_quads.end(), [](const sQuad& q1, const sQuad& q2)
        {
            float z1 = (q1.points[0].z + q1.points[1].z + q1.points[2].z + q1.points[3].z) * 0.25f;
            float z2 = (q2.points[0].z + q2.points[1].z + q2.points[2].z + q2.points[3].z) * 0.25f;
            return z1 > z2;
        });

        // Draw quad containing mouse pointer
        if(!mouse_quads.empty()) {
            mouse_cell = mouse_quads.front().cell;
            mouse_face = mouse_quads.front().face;
            DrawWarpedDecal(rendSelect.decal, {{mouse_quads.front().points[0].x, mouse_quads.front().points[0].y},
                                               {mouse_quads.front().points[1].x, mouse_quads.front().points[1].y},
                                               {mouse_quads.front().points[2].x, mouse_quads.front().points[2].y},
                                               {mouse_quads.front().points[3].x, mouse_quads.front().points[3].y}});
        }

        SetDrawTarget(menu_layer);
        Clear(olc::BLANK);
        man.Draw(menu_sprites.sprite, { ScreenWidth()/2,ScreenHeight()/2 });
        SetDrawTarget(editor_layer);

        // 7) Draw some debug info
        DrawStringDecal({ 0,0 }, "Cursor: " + std::to_string(vCursor.x) + ", " + std::to_string(vCursor.y), olc::YELLOW, { 0.5f, 0.5f });
        DrawStringDecal({ 0,6 }, "Angle: " + std::to_string(fCameraAngle), olc::YELLOW, { 0.5f, 0.5f });
        DrawStringDecal({ 0,12 }, "Pitch: " + std::to_string(fCameraPitch), olc::YELLOW, { 0.5f, 0.5f });
        DrawStringDecal({ 0,18 }, "Zoom: " + std::to_string(fCameraZoom), olc::YELLOW, { 0.5f, 0.5f });
        DrawStringDecal({ 0,24 }, "MouseXY: " + std::to_string(GetMouseX()) + ", " + std::to_string(GetMouseY()), olc::YELLOW, { 0.5f, 0.5f });
        DrawStringDecal({ 0,30 }, "Mouse Cell: " + std::to_string(mouse_cell.x) + ", " + std::to_string(mouse_cell.y), olc::YELLOW, { 0.5f, 0.5f });
        DrawStringDecal({ 0,36 }, "Mouse Face: " + face_names[mouse_face], olc::YELLOW, {0.5f, 0.5f });
        DrawStringDecal({ 0,42 }, sLastAction, olc::YELLOW, { 0.5f, 0.5f });


        // Graceful exit if user is in full screen mode
        return !GetKey(olc::Key::ESCAPE).bPressed;
    }
};

namespace boost {
    namespace serialization {
        template<class Archive>
        void serialize(Archive &ar, olc::v2d_generic<int32_t>& s, const unsigned int version) {
            ar & s.x;
            ar & s.y;
        }
    }
}
