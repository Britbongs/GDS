#ifndef GAME_BLACKBOARD_H
#define GAME_BLACKBOARD_H

#include "Krawler.h"

//physics engine & game
const static Krawler::Rectf SCENE_BOUNDS(-1000, -1000, 2000, 2000);

const static float METRES_TO_PIXELS = 900;

//planet & gravity
const static float PLANET_RADIUS = 32.0f;

constexpr float MINIMUM_PLANET_DENSITY = 1.0e25f / (32000.0f*32000.0f); //kg/m^3

const static float GRAVITATIONAL_CONSTANT = 6.67e-11;

const static float GRATIVTY_BALANCE = 3.0f;

//projectile 
const static float CACHED_PROJECTILE_COUNT = 10;

const static float PROJECTILE_MASS = 5.9e4f;

const static float PROJECTILE_RADIUS = 8.0f;

const static float KICKOFF_FORCE = 1.9e12f;

const static float PROJECTILE_ALIVE_TIME = 10.0f;
#endif