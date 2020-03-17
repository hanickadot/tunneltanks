#include "base.h"

char TANK_SPRITE[9][7][7] = {
	{{0,0,0,2,0,0,0},
	 {0,3,0,1,2,0,0},
	 {0,0,3,1,1,2,0},
	 {2,1,1,3,1,1,2},
	 {0,2,1,1,1,0,0},
	 {0,0,2,1,0,0,0},
	 {0,0,0,2,0,0,0}},
	
	{{0,0,0,3,0,0,0},
	 {0,2,0,3,0,2,0},
	 {0,2,1,3,1,2,0},
	 {0,2,1,3,1,2,0},
	 {0,2,1,1,1,2,0},
	 {0,2,1,1,1,2,0},
	 {0,2,0,0,0,2,0}},
	
	{{0,0,0,2,0,0,0},
	 {0,0,2,1,0,3,0},
	 {0,2,1,1,3,0,0},
	 {2,1,1,3,1,1,2},
	 {0,0,1,1,1,2,0},
	 {0,0,0,1,2,0,0},
	 {0,0,0,2,0,0,0}},
	
	{{0,0,0,0,0,0,0},
	 {0,2,2,2,2,2,2},
	 {0,0,1,1,1,1,0},
	 {3,3,3,3,1,1,0},
	 {0,0,1,1,1,1,0},
	 {0,2,2,2,2,2,2},
	 {0,0,0,0,0,0,0}},
	
	{{0,0,0,0,0,0,0},
	 {0,0,0,0,0,0,0},
	 {1,0,2,2,2,3,0},
	 {1,0,2,0,2,3,0}, /* << This direction doesn't get used. */
	 {1,1,2,2,2,3,3},
	 {0,0,0,0,0,0,0},
	 {0,0,0,0,0,0,0}},
	
	{{0,0,0,0,0,0,0},
	 {2,2,2,2,2,2,0},
	 {0,1,1,1,1,0,0},
	 {0,1,1,3,3,3,3},
	 {0,1,1,1,1,0,0},
	 {2,2,2,2,2,2,0},
	 {0,0,0,0,0,0,0}},
	
	{{0,0,0,2,0,0,0},
	 {0,0,2,1,0,0,0},
	 {0,2,1,1,1,0,0},
	 {2,1,1,3,1,1,2},
	 {0,0,3,1,1,2,0},
	 {0,3,0,1,2,0,0},
	 {0,0,0,2,0,0,0}},
	
	{{0,2,0,0,0,2,0},
	 {0,2,1,1,1,2,0},
	 {0,2,1,1,1,2,0},
	 {0,2,1,3,1,2,0},
	 {0,2,1,3,1,2,0},
	 {0,2,0,3,0,2,0},
	 {0,0,0,3,0,0,0}},
	
	{{0,0,0,2,0,0,0},
	 {0,0,0,1,2,0,0},
	 {0,0,1,1,1,2,0},
	 {2,1,1,3,1,1,2},
	 {0,2,1,1,3,0,0},
	 {0,0,2,1,0,3,0},
	 {0,0,0,2,0,0,0}}
};
