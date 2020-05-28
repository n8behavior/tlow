#ifndef AIROGUE_ACTOR_H
#define AIROGUE_ACTOR_H

//#include "libtcod/libtcod.hpp"

struct Actor {
    int x {0}, y {0}; // position on the map
    int ch {'@'}; // ascii code
    //TCODColor col {TCODColor::yellow};

    //Actor(int x, int y, int ch, const TCODColor &col);
    void render() const;
};

#endif // AIROGUE_ACTOR_H
