#pragma once

#include <drawbuffer.h>
#include <types.h>
#include "tweak.h"
#include <memory>
#include <array>
#include <vector>

#include "bitmaps.h"
#include "parallelism.h"

enum class BaseCollision
{
	None,
	Yours,
	Enemy,
};

enum class LevelVoxel : char
{
	Blank = ' ',
	DirtHigh = 'D',
	DirtLow = 'd',
	DirtGrow = 'g',
	Rock = 'r',
	DecalHigh = '.',
	DecalLow = ',',
	BaseMin = '0', // goes up to '7' for various tank colors
	BaseMax = '7',

	LevelGenDirt = 0,
	LevelGenRock = 1,
	LevelGenMark = 2,
};
class Voxels
{
public:
	static bool IsDirt(LevelVoxel voxel) { return voxel == LevelVoxel::DirtHigh || voxel == LevelVoxel::DirtLow; }
	static bool IsDiggable(LevelVoxel voxel) { return voxel == LevelVoxel::DirtHigh || voxel == LevelVoxel::DirtLow || voxel == LevelVoxel::DirtGrow; }
	static bool IsSoftCollision(LevelVoxel voxel) { return IsDirt(voxel); }
	static bool IsBlockingCollision(LevelVoxel voxel) { return voxel == LevelVoxel::Rock || (voxel >= LevelVoxel::BaseMin && voxel <= LevelVoxel::BaseMax); }
	static bool IsAnyCollision(LevelVoxel voxel) { return IsSoftCollision(voxel) || IsBlockingCollision(voxel); }
	static bool IsBase(LevelVoxel voxel) { return (voxel >= LevelVoxel::BaseMin && voxel <= LevelVoxel::BaseMax); }
	static bool IsScorched(LevelVoxel voxel) { return voxel == LevelVoxel::DecalHigh || voxel == LevelVoxel::DecalLow; }
};

class TankBase
{
	Position position;
public:
	TankBase(Position position) : position(position) { }
	Position GetPosition() { return this->position; }
};



class LevelData
{
	using Container = ValueArray<LevelVoxel>;
public:
	Container array;

	LevelData(Size size);
	
	LevelVoxel& operator[](int i) { return array[i]; }
	const LevelVoxel& operator[](int i) const { return array[i]; }

	Container::iterator begin() { return array.begin(); }
	Container::iterator end() { return array.end(); }
	Container::const_iterator cbegin() const { return array.cbegin(); }
	Container::const_iterator cend() const { return array.cend(); }
};

class Level
{
public:
	
private:
	LevelData data;
	Size size;
	DrawBuffer* drawBuffer;
	std::vector<std::unique_ptr<TankBase>> spawn;

public:
	Level(Size size, DrawBuffer* db);

	Size GetSize() const { return size; };

	/* Voxel get-set-reference operations */
	void SetVoxel(Position pos, LevelVoxel voxel);
	LevelVoxel GetVoxel(Position pos) const;
	LevelVoxel& Voxel(Position pos);

	void SetVoxelRaw(Position pos, LevelVoxel voxel);
	void SetVoxelRaw(int offset, LevelVoxel voxel);
	LevelVoxel GetVoxelRaw(Position pos) const;
	LevelVoxel GetVoxelRaw(int offset) const;
	LevelVoxel& VoxelRaw(Position pos);

	/* Draw buffer interaction */
	void CommitPixel(Position pos) const;
	void CommitAll() const;
	void DumpBitmap(const char* filename) const;

	/* Color lookup. Can be somewhere else. */
	static Color GetVoxelColor(LevelVoxel voxel);
	
	/* Count neighbors is used when level building and for ad-hoc queries (e.g. dirt regeneration) */
	int CountNeighborValues(Position pos);
	int CountNeighbors(Position pos, LevelVoxel neighbor_value);
     template <typename CountFunc>
	int CountNeighbors(Position pos, CountFunc count_func);

	/* Level generation */
	void GenerateDirtAndRocks();
	void CreateBases();
	 template <typename VoxelFunc>
	void ForEachVoxel(VoxelFunc func);
	 template <typename VoxelFunc>
	void ForEachVoxelParallel(VoxelFunc func, WorkerCount worker_count = {});

	/* Tank-related stuff */
	TankBase* GetSpawn(TankColor color) const;
	void SetSpawn(TankColor color, std::unique_ptr<TankBase>&& tank_base);
	void SetSpawn(TankColor color, Position position); 
	bool DigHole(Position pos);
	BaseCollision CheckBaseCollision(Position pos, TankColor color);
private:
	bool IsInBounds(Position pos) const;
	
	void CreateBase(Position pos, TankColor color);
};

template <typename VoxelFunc> // requires{ voxelFunc(Position, LevelVoxel&) -> void; }
void Level::ForEachVoxel(VoxelFunc voxelFunc) 
{
	Position pos;
	for (pos.x = 0; pos.x < this->GetSize().x; ++pos.x)
		for (pos.y = 0; pos.y < this->GetSize().y; ++pos.y)
		{
			voxelFunc(pos, this->Voxel(pos));
		}
}
template <typename VoxelFunc> 
void Level::ForEachVoxelParallel(VoxelFunc voxelFunc, WorkerCount worker_count)
{
	auto parallel_slice = [this, voxelFunc](int min, int max, ThreadLocal* threadLocal)
	{
		Position pos;
		for (pos.x = min; pos.x <= max; ++pos.x)
			for (pos.y = 0; pos.y < this->GetSize().y; ++pos.y)
			{
				voxelFunc(pos, this->Voxel(pos), threadLocal);
			}
		return 0;
	};

	parallel_for(parallel_slice, 0, this->GetSize().x - 1, worker_count);
}

template <typename CountFunc>
int Level::CountNeighbors(Position pos, CountFunc count_func)
{
	return count_func(GetVoxelRaw({ pos.x - 1 + GetSize().x * (pos.y - 1) })) +
		count_func(GetVoxelRaw({ pos.x + GetSize().x * (pos.y - 1) })) +
		count_func(GetVoxelRaw({ pos.x + 1 + GetSize().x * (pos.y - 1) })) +
		count_func(GetVoxelRaw({ pos.x - 1 + GetSize().x * (pos.y) })) +
		count_func(GetVoxelRaw({ pos.x + 1 + GetSize().x * (pos.y) })) +
		count_func(GetVoxelRaw({ pos.x - 1 + GetSize().x * (pos.y + 1) })) +
		count_func(GetVoxelRaw({ pos.x + GetSize().x * (pos.y + 1) })) +
		count_func(GetVoxelRaw({ pos.x + 1 + GetSize().x * (pos.y + 1) }));
}
