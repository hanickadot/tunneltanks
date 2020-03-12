#pragma once
#include <SDL.h>
#include <tank.h>

struct ControllerOutput 
{
	Speed speed = { };
	bool is_shooting = false;
};

class Controller
{
public:
	virtual ~Controller() = default;
	virtual ControllerOutput ApplyControls(struct PublicTankInfo* tankPublic) = 0;
};


class KeyboardController : public Controller
{
	SDLKey left, right, up, down, shoot;
public:
	KeyboardController(SDLKey left, SDLKey right, SDLKey up, SDLKey down, SDLKey shoot);
	ControllerOutput ApplyControls(PublicTankInfo* tankPublic) override;

};

/* The SDL-based keyboard controller: */
class JoystickController : public Controller
{
	SDL_Joystick* joystick;
public:
	JoystickController();
	ControllerOutput ApplyControls(PublicTankInfo* tankPublic) override;
};

