#include <cstdlib>

#include <controller.h>
#include <tank.h>
#include <memalloc.h>
#include <random.h>
#include <tweak.h>
#include "levelslice.h"


/* Used when seeking a base entrance: */
#define OUTSIDE (BASE_SIZE/2 + 5)


/* Our first AI: Twitch! (note: the 'I' in 'AI' is being used VERY loosely) */

/* The different Twitch travel modes: */
typedef enum TwitchMode {
	TWITCH_START,     /* An init state that picks a direction to leave from. */
	TWITCH_EXIT_UP,   /* Leave the base in an upward direction. */
	TWITCH_EXIT_DOWN, /* Leave the base in a downward direction. */
	TWITCH_TWITCH,    /* Do what Twitch does best. */
	TWITCH_RETURN,    /* Return to base. (Low fuel/health.) */
	TWITCH_RECHARGE   /* Seek to middle of base, and wait til fully healed. */
} TwitchMode;

typedef struct TwitchPrivateData {
	int      vx, vy, s;
	int time_to_change;
	TwitchMode    mode;
} TwitchPrivateData;


static void do_start(PublicTankInfo *i, void *d, int *vx, int *vy, int *s) {
	TwitchPrivateData *data = static_cast<TwitchPrivateData*>(d);
	int no_up, no_down;
	
	no_up   = level_slice_query_circle(i->slice, 0, -OUTSIDE+1) == LSQ_COLLIDE;
	no_down = level_slice_query_circle(i->slice, 0,  OUTSIDE-1) == LSQ_COLLIDE;
	
	if(no_up && no_down) {
		/* TODO: Make it so that this condition isn't possible... */
		data->mode = Random::Bool(500) ? TWITCH_EXIT_UP : TWITCH_EXIT_DOWN;
	} else if(no_up) {
		data->mode = TWITCH_EXIT_DOWN;
	} else if(no_down) {
		data->mode = TWITCH_EXIT_UP;
	} else
		data->mode = Random::Bool(500) ? TWITCH_EXIT_UP : TWITCH_EXIT_DOWN;
}

static void do_exit_up(PublicTankInfo *i, void *d, int *vx, int *vy, int *s) {
	TwitchPrivateData *data = static_cast<TwitchPrivateData*>(d);
	
	*vx = *vy = 0;
	*s = 0;
	
	if(i->y < -OUTSIDE) { /* Some point outside the base. */
		data->time_to_change = 0;
		data->mode = TWITCH_TWITCH;
		return;
	}
	
	*vy = -1;
}

static void do_exit_down(PublicTankInfo *i, void *d, int *vx, int *vy, int *s) {
	TwitchPrivateData *data = static_cast<TwitchPrivateData*>(d);
	
	*vx = *vy = 0;
	*s = 0;
	
	if(i->y > OUTSIDE) {
		data->time_to_change = 0;
		data->mode = TWITCH_TWITCH;
		return;
	}
	
	*vy = 1;	
}

static void do_twitch(PublicTankInfo *i, void *d, int *vx, int *vy, int *s) {
	TwitchPrivateData *data = static_cast<TwitchPrivateData*>(d);
	
	if(i->health < TANK_STARTING_SHIELD/2 || i->energy < TANK_STARTING_FUEL/3 ||
	  (abs(i->x) < BASE_SIZE/2 && abs(i->y) < BASE_SIZE/2) ) {
		/* We need a quick pick-me-up... */
		data->mode = TWITCH_RETURN;
	}
	
	if(!data->time_to_change) {
		data->time_to_change = Random::Int(10u, 30u);
		data->vx = Random::Int(0,2) - 1;
		data->vy = Random::Int(0,2) - 1;
		data->s  = Random::Bool(300);
	}
	
	data->time_to_change--;
	*vx = data->vx;
	*vy = data->vy;
	*s  = data->s;
}

/* Make a simple effort to get back to your base: */
static void do_return(PublicTankInfo *i, void *d, int *vx, int *vy, int *s) {
	TwitchPrivateData *data = static_cast<TwitchPrivateData*>(d);
	
	/* Seek to the closest entrance: */
	int targety = (i->y < 0) ? -OUTSIDE : OUTSIDE;
	
	/* Check to see if we've gotten there: */
	if((i->x == 0 && i->y == targety) || 
	   (abs(i->x)<BASE_SIZE/2 && abs(i->y)<BASE_SIZE/2)) {
		*s = 0;
		*vx = *vy = 0;
		data->mode = TWITCH_RECHARGE;
		return;
	}
	
	/* If we are close to the base, we need to navigate around the walls: */
	if( abs(i->x) <= OUTSIDE && abs(i->y) < OUTSIDE ) {
		*s = 0;
		*vx = 0;
		*vy = (i->y < targety) * 2 - 1;
		return;
	}
	
	/* Else, we will very simply seek to the correct point: */
	*s = 0;
	*vx = i->x != 0       ? ((i->x < 0) * 2 - 1) : 0;
	*vy = i->y != targety ? ((i->y < targety) * 2 - 1) : 0;
}

static void do_recharge(PublicTankInfo *i, void *d, int *vx, int *vy, int *s) {
	TwitchPrivateData *data = static_cast<TwitchPrivateData*>(d);
	
	/* Check to see if we're fully charged/healed: */
	if(i->health == TANK_STARTING_SHIELD && i->energy == TANK_STARTING_FUEL) {
		data->mode = TWITCH_START;
		return;
	}
	
	/* Else, seek to the base's origin, and wait: */
	*s = 0;
	*vx = i->x ? ((i->x < 0) * 2 - 1) : 0;
	*vy = i->y ? ((i->y < 0) * 2 - 1) : 0;
}

static void twitch_controller(PublicTankInfo *i, void *d, int *vx, int *vy, int *s) {
	TwitchPrivateData *data = static_cast<TwitchPrivateData*>(d);
	
	switch(data->mode) {
		case TWITCH_START:     do_start    (i, d, vx, vy, s); return;
		case TWITCH_EXIT_UP:   do_exit_up  (i, d, vx, vy, s); return;
		case TWITCH_EXIT_DOWN: do_exit_down(i, d, vx, vy, s); return;
		case TWITCH_TWITCH:    do_twitch   (i, d, vx, vy, s); return;
		case TWITCH_RETURN:    do_return   (i, d, vx, vy, s); return;
		case TWITCH_RECHARGE:  do_recharge (i, d, vx, vy, s); return;
	}
}

void controller_twitch_attach( Tank *t ) {
	auto  data = std::make_shared<TwitchPrivateData>();
	data->mode = TWITCH_START;
	
	t->SetController(twitch_controller, std::static_pointer_cast<void>(data));
}

