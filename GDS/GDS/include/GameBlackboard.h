#ifndef GAME_BLACKBOARD_H
#define GAME_BLACKBOARD_H

#include "Krawler.h"

//physics engine & game
const static Krawler::Rectf SCENE_BOUNDS(-1000, -1000, 2000, 2000);

const static float METRES_TO_PIXELS = 900;

//planet & gravity
const static float PLANET_RADIUS = 32.0f;

constexpr static Krawler::int32 EXTRA_PLANET_COUNT = 3;

constexpr float MINIMUM_PLANET_DENSITY = 1.0e25f / (32000.0f*32000.0f); //kg/m^3

constexpr float MAX_PLANET_DENSITY = 1.0e26f / (32000.0f*32000.0f); //kg/m^3

const static float GRAVITATIONAL_CONSTANT = 6.67e-11f;

const static float GRATIVTY_BALANCE = 3.2f;

//projectile 
const static Krawler::uint32 CACHED_PROJECTILE_COUNT = 15;

const static float PROJECTILE_MASS = 5.9e4f;

const static float PROJECTILE_RADIUS = 8.0f;

const static float KICKOFF_FORCE = 1.4e12f;

const static float PROJECTILE_ALIVE_TIME = 20.0f;

//targets count 
constexpr static Krawler::int32 TARGET_COUNT = EXTRA_PLANET_COUNT * 3;

constexpr static Krawler::int32 TARGETS_PER_PLANET = 3;

const static float TARGET_SIZE = 32;

#endif