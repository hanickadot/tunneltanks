#include "world.h"
#include "random.h"


void World::Advance(class DrawBuffer* draw_buffer)
{
	RegrowPass();
	
	/* Clear everything: */
	this->tank_list->for_each([=](Tank* t) {t->Clear(draw_buffer); });
	this->projectiles->Erase(draw_buffer, this->level.get());

	/* Charge a small bit of energy for life: */
	this->tank_list->for_each([=](Tank* t) {t->AlterEnergy(TANK_IDLE_COST); });

	/* See if we need to be healed: */
	this->tank_list->for_each([=](Tank* t) {t->TryBaseHeal(); });

	/* Move everything: */
	this->projectiles->Advance(this->level.get(), this->tank_list.get());
	this->tank_list->for_each([=](Tank* t) {t->DoMove(this->tank_list.get()); });

	/* Draw everything: */
	this->projectiles->Draw(draw_buffer);
	this->tank_list->for_each([=](Tank* t) {t->Draw(draw_buffer); });
}

void World::RegrowPass()
{
	Stopwatch<> elapsed;
	int holes_decayed = 0;
	int dirt_grown = 0;
	this->level->ForEachVoxel([this, &holes_decayed, &dirt_grown](Position pos, LevelVoxel& vox)
	{
		if (vox == LevelVoxel::Blank)
		{
			int neighbors = this->level->CountNeighbors(pos, [](auto voxel) { return Voxels::IsDirt(voxel) ? 1 : 0; });
			if (neighbors > 2 && Random.Int(0, 10000) < tweak::DirtRegrowSpeed * neighbors) {

				vox = LevelVoxel::DirtGrow;
				this->level->CommitPixel(pos);
				++holes_decayed;
			}
		}
		else if (vox == LevelVoxel::DirtGrow)
		{
			if (Random.Int(0, 1000) < tweak::DirtRecoverSpeed) {
				vox = Random.Bool(500) ? LevelVoxel::DirtHigh : LevelVoxel::DirtLow;
				this->level->CommitPixel(pos);
				++dirt_grown;
			}
		}
	}
	);
	this->regrow_elapsed += elapsed.GetElapsed();
}
