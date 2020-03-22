#pragma once

#include <thread>
#include <algorithm>
#include <types.h>
#include <chrono>

namespace tweak {
/*
 * Most of the game engine's arbitrary limits are stored in here:
 */
	namespace perf {
		constexpr int parallelism_percent = 100;
		inline unsigned int parallelism_degree = std::max(1u, std::thread::hardware_concurrency() * parallelism_percent / 100);

		/* The desired speed in frames per second: */
		constexpr int TargetFps = 24;
		constexpr std::chrono::milliseconds AdvanceStep{ 1000 / TargetFps };

	}

    constexpr int DirtRecoverSpeed = 2; /* Average delay before growing finishes and new dirt is formed. More is faster. */
    constexpr int DirtRegrowSpeed = 5; /* Average delay before it starts growing back. More is faster.*/

	namespace screen
	{
		/* The default size of the window: */
		constexpr Size size = { 640, 400 };
		constexpr int max_windows = 4;
		constexpr int max_status = 4;
		constexpr int max_bitmaps = 4;

		/* Various attributes of the status bar: */
		constexpr int status_height = 11;
		constexpr int status_border = 1;
	}


/* Window title / version string: */
#define WINDOW_TITLE                   "TunnelTanks"
#define VERSION                        "0.5 alpha"

/* The virtual resolution of the game. (IE: How many blocks tall/wide) */
#define GAME_WIDTH                     160
#define GAME_HEIGHT                    100
constexpr Size GameSize = { GAME_WIDTH, GAME_HEIGHT };


/* The minimum distance between two tanks in the world. If this is set too high,
 * then the level generator may start throwing exceptions: */
#define MIN_SPAWN_DIST                 150

 /* The maximum number of tanks: */
constexpr int MaxPlayers = 8;

namespace tank {

	constexpr int MaxLives = 3;
	constexpr int RespawnDelay = perf::TargetFps * 3;
	
	/* The number of frames to wait in between shots: */
	constexpr int BulletDelay = 3;
	/* The maximum number of bullets allowed from a given tank: */
	constexpr int BulletMax = 6;
	/* The speed in pixels/frame of bullets: */
	constexpr int BulletSpeed = 3;

	/* Various constants for energy calculation: */
	constexpr int StartingFuel = 24000;
	constexpr int ShootCost = -160;
	constexpr int MoveCost = -8;
	constexpr int IdleCost = -3;
	constexpr int HomeChargeSpeed = 300;
	constexpr int EnemyChargeSpeed = 90;
				  
	/* Various constants for health calculation: */
	constexpr int StartingShield = 1000;
	constexpr int ShotDamage = -160;
	constexpr int HomeHealSpeed = 3;

    constexpr int TurretLength = 3;

}
/* Constants for drawing static: (The bottom 3 constants are out of 1000) */
#define STATIC_THRESHOLD               (tweak::tank::StartingFuel/5)
#define STATIC_TRANSPARENCY            200
#define STATIC_BLACK_BAR_ODDS          500
#define STATIC_BLACK_BAR_SIZE          500

/* Various base sizes: */
#define BASE_SIZE                      35
#define BASE_DOOR_SIZE                 7

namespace explosion::dirt
{
    constexpr int ShrapnelCount = 10;
    constexpr int Speed = 12;
    constexpr int Frames = 10;
} // namespace explosion::dirt
namespace explosion::normal
{
    constexpr int ShrapnelCount = 14;
    constexpr int Speed = 18;
    constexpr int Frames = 13;
} // namespace explosion::normal
namespace explosion::death
{
    constexpr int ShrapnelCount = 100;
    constexpr int Speed = 8;
    constexpr int Frames = 72;
} // namespace explosion::death
/* Characters used in level structures for things: */

/* Default to keeping memory stats: */
#ifndef _MEM_STATS
#define _MEM_STATS
#endif

 
}