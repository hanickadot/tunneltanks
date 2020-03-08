#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <gamelib.h>
#include <levelgen.h>
#include <levelgenutil.h>
#include <level.h>
#include <memalloc.h>
#include <random.h>

#include <deque>

namespace levelgen::toast {

/* Configuration Constants: */
constexpr int BORDER = 30;
constexpr int FILTER = 70;
constexpr int ODDS = 300;
constexpr int FILLRATIO = 65;
constexpr int TREESIZE = 150;

typedef struct Pairing {
	int dist, a, b;
} Pairing;

#ifdef _TESTING
static void level_draw_ascii(Level *lvl) {
	int x,y;
	
	for(y=0; y<lvl->height; y++) {
		for(x=0; x<lvl->width; x++)
			printf("%c", lvl->array[ y*lvl->width + x ]?'#':' ');
		printf("\n");
	}
}
#endif /* _TESTING */

/*----------------------------------------------------------------------------*
 * STAGE 1: Generate a random tree                                            *
 *----------------------------------------------------------------------------*/

static int pairing_cmp(const void *a, const void *b) {
	return ((Pairing *)a)->dist - ((Pairing *)b)->dist;
}

static void generate_tree(Level *lvl) {
	int *dsets, paircount;
	int i, j;
	int k;
	Position *points;
	Pairing *pairs;
	
	/* Get an array of disjoint set IDs: */
	dsets = static_cast<int*>(get_mem( sizeof(int) * TREESIZE ));
	for(i=0; i<TREESIZE; i++) dsets[i] = i;
	
	/* Randomly generate all points: */
	points = static_cast<Position*>(get_mem( sizeof(Position) * TREESIZE ));
	for(i=0; i<TREESIZE; i++) points[i] = pt_rand(lvl->GetSize(), BORDER);
	
	/* While we're here, copy in some of those points: */
	lvl->SetSpawn(0, points[0]);
	for(i=1,j=1; i<TREESIZE && j<MAX_TANKS; i++) {
		for(k=0; k<j; k++) {
			if(pt_dist(points[i],lvl->GetSpawn(k)) < MIN_SPAWN_DIST*MIN_SPAWN_DIST)
				break;
		}
		
		if(k!=j) continue;
		lvl->SetSpawn(j++, points[i]);
	}
	if(j!=MAX_TANKS) {
		/* TODO: More robust error handling. */
		gamelib_error("OH SHUCKS OH SHUCKS OH SHUCKS\n");
		exit(1);
	}
	/* Get an array of all point-point pairings: */
	paircount = TREESIZE*(TREESIZE-1) / 2;
	pairs = static_cast<Pairing*>(get_mem( sizeof(Pairing) * (paircount) ));

	/* Set up all the pairs, and sort them: */
	for(k=i=0; i<TREESIZE; i++)
		for(j=i+1; j<TREESIZE; j++, k++) {
			pairs[k].a = i; pairs[k].b = j;
			pairs[k].dist = pt_dist(points[i], points[j]);
		}
	qsort(pairs, paircount, sizeof(Pairing), pairing_cmp);
	for(i=j=0; i<paircount; i++) {
		int aset, bset;
		
		/* Trees only have |n|-1 edges, so call it quits if we've selected that
		 * many: */
		if(j>=TREESIZE-1) break;
		
		aset = dsets[pairs[i].a]; bset = dsets[pairs[i].b];
		if(aset == bset) continue;
		
		/* Else, these points are in different disjoint sets. "Join" them by
		 * drawing them, and merging the two sets: */
		j+=1;
		for(k=0; k<TREESIZE; k++) 
			if(dsets[k] == bset) 
				dsets[k] = aset;
		draw_line(lvl, points[pairs[i].a], points[pairs[i].b], 0, 0);
	}
	
	/* We don't need this data anymore: */
	free_mem(pairs);
	free_mem(points);
	free_mem(dsets);
}


/*----------------------------------------------------------------------------*
 * STAGE 2: Randomly expand upon the tree                                     *
 *----------------------------------------------------------------------------*/

/* Some cast-to-int tricks here may be fun... ;) */
static int has_neighbor(Level *lvl, int x, int y) {
	if (!lvl->GetVoxelRaw({ x - 1, y - 1})) return 1;
	if (!lvl->GetVoxelRaw({ x,    y - 1 })) return 1;
	if (!lvl->GetVoxelRaw({ x + 1, y - 1 })) return 1;
	if (!lvl->GetVoxelRaw({ x - 1, y  })) return 1;
	if (!lvl->GetVoxelRaw({ x + 1, y  })) return 1;
	if (!lvl->GetVoxelRaw({ x - 1, y + 1 })) return 1;
	if (!lvl->GetVoxelRaw({ x,     y + 1 })) return 1;
	if (!lvl->GetVoxelRaw({ x + 1, y + 1 })) return 1;
	return 0;
}

static void set_outside(Level *lvl, char val) {
	int i;
	Size size = lvl->GetSize();
	
	for (i = 0; i < size.x;   i++) lvl->VoxelRaw({ i, 0 }) = val;
	for (i = 0; i < size.x;   i++) lvl->VoxelRaw({ i, size.y - 1 }) = val;
	for (i = 1; i < size.y-1; i++) lvl->VoxelRaw({ 0, i }) = val;
	for (i = 1; i < size.y-1; i++) lvl->VoxelRaw({ size.x - 1, i }) = val;
}

static void expand_init(Level *lvl, std::deque<Position>& q) {
	int x, y;
	
	for(y=1; y<lvl->GetSize().y-1; y++)
		for(x=1; x<lvl->GetSize().x-1; x++)
			if (lvl->GetVoxelRaw({ x, y }) && has_neighbor(lvl, x, y)) {
				lvl->SetVoxelRaw({ x, y }, 2);
				q.emplace_back(x,y);
			}
}

#define MIN2(a,b)   ((a<b) ? a : b)
#define MIN3(a,b,c) ((a<b) ? a : (b<c) ? b : c)
static int expand_once(Level *lvl, std::deque<Position>& q) {
	Position temp;
	int j, count = 0;
	
	size_t total = q.size();
	for(size_t i=0; i<total; i++) {
		int xodds, yodds, odds;
		
		temp = q.front();
		q.pop_front();

		xodds = ODDS * MIN2(lvl->GetSize().x - temp.x, temp.x) / FILTER;
		yodds = ODDS * MIN2(lvl->GetSize().y - temp.y, temp.y) / FILTER;
		odds  = MIN3(xodds, yodds, ODDS);
		
		if(Random::Bool(odds)) {
			lvl->SetVoxelRaw(temp, 0);
			count++;
			
			/* Now, queue up any neighbors that qualify: */
			for(j=0; j<9; j++) {
				char *c;
				int tx, ty;
				
				if(j==4) continue;
				
				tx = temp.x + (j%3) - 1; ty = temp.y + (j/3) - 1;
				c = &lvl->VoxelRaw({ tx, ty });
				if(*c == 1) {
					*c = 2;
					q.emplace_back(Position{ tx, ty });
				}
			}
		} else
			q.emplace_back(temp);
	}
	return count;
}

static void expand_cleanup(Level *lvl) {

	lvl->ForEachVoxel([](LevelVoxel& voxel) { voxel = !!voxel; });
}

static void randomly_expand(Level *lvl) {
	int cur = 0, goal = lvl->GetSize().x * lvl->GetSize().y * FILLRATIO / 100;
	
	/* Experimentally, the queue never grew to larger than 3/50ths of the level
	 * size, so we can use that to save quite a bit of memory: */
	auto queue = std::deque<Position>(lvl->GetSize().x * lvl->GetSize().y * 3 / 50);
	
	expand_init(lvl, queue);
	while( (cur += expand_once(lvl, queue)) < goal );
	expand_cleanup(lvl);
}


/*----------------------------------------------------------------------------*
 * STAGE 3: Smooth out the graph with a cellular automaton                    *
 *----------------------------------------------------------------------------*/

static int count_neighbors(Level* lvl, int x, int y) {
	return lvl->GetVoxelRaw({ x - 1, y - 1 }) +
		lvl->GetVoxelRaw({ x,   y - 1 }) +
		lvl->GetVoxelRaw({ x + 1, y - 1 }) +
		lvl->GetVoxelRaw({ x - 1, y }) +
		lvl->GetVoxelRaw({ x + 1, y }) +
		lvl->GetVoxelRaw({ x - 1, y + 1 }) +
		lvl->GetVoxelRaw({ x,   y + 1 }) +
		lvl->GetVoxelRaw({ x + 1, y + 1 });
}
	
#define MIN2(a,b)   ((a<b) ? a : b)
#define MIN3(a,b,c) ((a<b) ? a : (b<c) ? b : c)
static int smooth_once(Level *lvl) {
	int x, y, count = 0;

	Size size = lvl->GetSize();
	for(y=1; y<size.y-1; y++)
		for(x=1; x<size.x-1; x++) {
			int n;
			LevelVoxel oldbit = lvl->GetVoxelRaw({ x, y });
			
			n = count_neighbors(lvl, x, y);
			lvl->SetVoxelRaw({ x, y }, oldbit ? (n >= 3) : (n > 4));
			
			count += lvl->GetVoxelRaw({ x, y }) != oldbit;
		}
	return count;
}

static void smooth_cavern(Level *lvl) {
	set_outside(lvl, 0);
	while(smooth_once(lvl));
	set_outside(lvl, 1);
}


/*----------------------------------------------------------------------------*
 * MAIN FUNCTIONS:                                                            *
 *----------------------------------------------------------------------------*/

void toast_generator(Level *lvl) {
	generate_tree(lvl);
	randomly_expand(lvl);
	smooth_cavern(lvl);
}

#ifdef _TESTING

int main() {
	clock_t t, all;
	Level lvl;
	int i;
	
	rand_seed();
	
	/* We don't need a full-fledged Level object, so let's just half-ass one: */
	lvl.width = 1000; lvl.height = 500;
	lvl.array = get_mem(sizeof(char) * lvl.width * lvl.height);
	for(i=0; i<lvl.width * lvl.height; i++) lvl.array[i] = 1;
	
	TIMER_START(all);
	TIMER_START(t);
	generate_tree(&lvl);
	TIMER_STOP(t);
	
	TIMER_START(t);
	randomly_expand(&lvl);
	TIMER_STOP(t);
	
	TIMER_START(t);
	smooth_cavern(&lvl);
	TIMER_STOP(t);
	
	printf("-----------\nTotal: ");
	TIMER_STOP(all);
	
	level_draw_ascii(&lvl);
	
	free_mem(lvl.array);
	
	print_mem_stats();
	return 0;
}
#endif /* _TESTING */

}
