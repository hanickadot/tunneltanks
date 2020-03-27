#include "base.h"
#include <cstdlib>

#include <level.h>
#include <random.h>
#include <tweak.h>
#include <types.h>
#include <drawbuffer.h>
#include <gamelib.h>

#include "exceptions.h"
#include <cassert>

#include "bitmap.h"
#include "colors.h"
#include "trace.h"


uint8_t LevelAdjacencyDataCompressed::operator[](int i) const
{
    uint8_t val = this->array[i / 2];
    if (i % 2 == 1)
        return val >> 4;
    return val & 0xF;
}

void LevelAdjacencyDataCompressed::Set(int i, uint8_t value)
{
    assert(value <= 0xF);
    uint8_t& val = this->array[i / 2];
    if (i % 2 == 1)
        val = (val & 0xF) | (value << 4);
    val = val & (0xF0 | (value & 0xF));
}


Level::Level(Size size, LevelDrawBuffer* draw_buffer)
	: size(size), drawBuffer (draw_buffer), data(size)
{
	std::fill(this->data.begin(), this->data.end(), LevelPixel::LevelGenRock);
}


void Level::SetLevelData(int i, LevelPixel value)
{
    this->data[i] = value;
}
void Level::SetLevelData(Position pos, LevelPixel value)
{
    this->data[pos.y * this->size.x + pos.x] = value;
}


void Level::SetVoxel(Position pos, LevelPixel voxel)
{
    assert(IsInBounds(pos));

    SetLevelData(pos.y*this->size.x + pos.x, voxel);
	CommitPixel(pos);
}

void Level::SetVoxelRaw(Position pos, LevelPixel voxel)
{
    SetLevelData(pos, voxel);
}

void Level::SetVoxelRaw(int offset, LevelPixel voxel)
{
	SetLevelData(offset, voxel);
}


//LevelPixel& Level::Voxel(Position pos)
//{
//    assert(IsInBounds(pos));
//    return this->data[pos.y * this->size.x + pos.x];
//}
//
//LevelPixel& Level::VoxelRaw(Position pos)
//{
//	return this->data[pos.y * this->size.x + pos.x];
//}

int Level::CountNeighborValues(Position pos)
{
	return (char)GetVoxelRaw({ pos.x - 1 + GetSize().x * (pos.y - 1) }) +
		   (char)GetVoxelRaw({ pos.x     + GetSize().x * (pos.y - 1) }) +
		   (char)GetVoxelRaw({ pos.x + 1 + GetSize().x * (pos.y - 1) }) +
		   (char)GetVoxelRaw({ pos.x - 1 + GetSize().x * (pos.y) }) +
		   (char)GetVoxelRaw({ pos.x + 1 + GetSize().x * (pos.y) }) +
		   (char)GetVoxelRaw({ pos.x - 1 + GetSize().x * (pos.y + 1) }) +
		   (char)GetVoxelRaw({ pos.x     + GetSize().x * (pos.y + 1) }) +
		   (char)GetVoxelRaw({ pos.x + 1 + GetSize().x * (pos.y + 1) });
}

int Level::CountNeighbors(Position pos, LevelPixel value)
{
	return !!(value == GetVoxelRaw({ pos.x - 1 + GetSize().x * (pos.y - 1) })) +
		   !!(value == GetVoxelRaw({ pos.x + GetSize().x * (pos.y - 1) })) +
		   !!(value == GetVoxelRaw({ pos.x + 1 + GetSize().x * (pos.y - 1) })) +
		   !!(value == GetVoxelRaw({ pos.x - 1 + GetSize().x * (pos.y) })) +
		   !!(value == GetVoxelRaw({ pos.x + 1 + GetSize().x * (pos.y) })) +
		   !!(value == GetVoxelRaw({ pos.x - 1 + GetSize().x * (pos.y + 1) })) +
		   !!(value == GetVoxelRaw({ pos.x + GetSize().x * (pos.y + 1) })) +
		   !!(value == GetVoxelRaw({ pos.x + 1 + GetSize().x * (pos.y + 1) }));
}



LevelPixel Level::GetVoxel(Position pos) const
{
	if (!IsInBounds(pos))
		return LevelPixel::Rock;
	return this->data[ pos.y * this->size.x + pos.x ];
}

LevelPixel Level::GetVoxelRaw(Position pos) const
{
	return this->data[pos.y * this->size.x + pos.x];
}

LevelPixel Level::GetVoxelRaw(int address) const
{
	return this->data[address];
}

void Level::GenerateDirtAndRocks()
{
    Position pos;
    for (pos.y = 0; pos.y < this->size.y; pos.y++)
        for (pos.x = 0; pos.x < this->size.x; pos.x++)
        {
			auto spot = this->GetVoxel(pos);
			if(spot != LevelPixel::LevelGenDirt)
				this->SetVoxel(pos, LevelPixel::Rock);
			else      
				this->SetVoxel(pos, Random.Bool(500) ? LevelPixel::DirtLow : LevelPixel::DirtHigh);
		}
}

void Level::CreateBase(Position pos, TankColor color)
{
	if(color >= tweak::MaxPlayers)
		return;
	
	for(int y = -BASE_SIZE / 2; y<=BASE_SIZE/2; y++) {
		for(int x = -BASE_SIZE / 2; x<=BASE_SIZE/2; x++) 
		{
			Position pix = pos + Offset{ x, y };
			if(abs(x) == BASE_SIZE/2 || abs(y) == BASE_SIZE/2) 
			{	// Outline
				if(x >= -BASE_DOOR_SIZE/2 && x <= BASE_DOOR_SIZE/2) 
					continue;

				SetVoxel(pix, static_cast<LevelPixel>(static_cast<char>(LevelPixel::BaseMin) + color));
			}
			else
				SetVoxel(pix, LevelPixel::Blank);
		}
	}
}

/* TODO: Rethink the method for adding bases, as the current method DEMANDS that
 *       you use MAX_TANKS tanks. */
void Level::CreateBases()
{
	for (TankColor i = 0; i < tweak::MaxPlayers; i++) {
		CreateBase({ this->spawn[i]->GetPosition().x, this->spawn[i]->GetPosition().y }, i);
	}
}

TankBase* Level::GetSpawn(TankColor color) const
{
	assert(color >= 0 && color < (int)this->spawn.size());
	return this->spawn[color].get();
}

void Level::SetSpawn(TankColor color, std::unique_ptr<TankBase>&& tank_base)
{
	assert(color >= 0 && color < tweak::MaxPlayers);
	if (TankColor(this->spawn.size()) <= color)
		this->spawn.resize(color + 1);
	this->spawn[color] = std::move(tank_base);
}

void Level::SetSpawn(TankColor color, Position position)
{
	this->SetSpawn(color, std::make_unique<TankBase>(position));
}

bool Level::DigHole(Position pos)
{
	bool digged = false;
	
	for(int ty = pos.y - 3; ty<= pos.y+3; ty++)
		for(int tx = pos.x - 3; tx<= pos.x+3; tx++) {
			/* Don't go out-of-bounds: */
			if (!Pixel::IsDiggable(GetVoxel({ tx, ty })))
				continue;
			
			/* Don't take out the corners: */
			if((tx==pos.x-3 || tx== pos.x+3) && (ty== pos.y-3 || ty== pos.y+3)) 
				continue;
			
			SetVoxel({ tx, ty }, LevelPixel::Blank);
			digged = true;
		}
	
	return digged;
}

void Level::CommitAll() const
{
	for (int y = 0; y < this->size.y; y++) {
		for (int x = 0; x < this->size.x; x++) {
			CommitPixel({ x, y });
		}
	}
}

bool Level::IsInBounds(Position pos) const
{
	return !(pos.x < 0 || pos.y < 0 || pos.x >= this->size.x || pos.y >= this->size.y);
}


void Level::CommitPixel(Position pos) const
{
	drawBuffer->SetPixel(pos, GetVoxelColor(this->GetVoxel(pos))); 
}


/* TODO: This needs to be done in a different way, as this approach will take 
 * MAX_TANKS^2 time to do all collision checks for all tanks. It should only
 * take MAX_TANKS time. */
BaseCollision Level::CheckBaseCollision(Position pos, TankColor color)
{
	for(TankColor id = 0; id < tweak::MaxPlayers; id++) {
		if(std::abs(this->spawn[id]->GetPosition().x - pos.x) < BASE_SIZE/2 && std::abs(this->spawn[id]->GetPosition().y - pos.y) < BASE_SIZE/2) {
			if (id == color)
				return BaseCollision::Yours;
			return BaseCollision::Enemy;
		}
	}
	
	return BaseCollision::None;
}

Color32 Level::GetVoxelColor(LevelPixel voxel)
{
	if (voxel == LevelPixel::DirtHigh)	     return Palette.Get(Colors::DirtHigh);
	else if (voxel == LevelPixel::DirtLow)   return Palette.Get(Colors::DirtLow);
	else if (voxel == LevelPixel::DirtGrow)  return Palette.Get(Colors::DirtGrow);
	else if (voxel == LevelPixel::Rock)      return Palette.Get(Colors::Rock);
	else if (voxel == LevelPixel::DecalLow)  return Palette.Get(Colors::DecalLow);
	else if (voxel == LevelPixel::DecalHigh) return Palette.Get(Colors::DecalHigh);
    else if (voxel == LevelPixel::ConcreteLow)  return Palette.Get(Colors::ConcreteLow);
    else if (voxel == LevelPixel::ConcreteHigh) return Palette.Get(Colors::ConcreteHigh);
	else if (voxel == LevelPixel::Blank)     return Palette.Get(Colors::Blank);
	else if (Pixel::IsBase(voxel))
		return Palette.GetTank(static_cast<char>(voxel) - static_cast<char>(LevelPixel::BaseMin))[0];
	else {
		assert(!"Unknown voxel.");
		return {};
	}
}

/* Dumps a level into a BMP file: */
void Level::DumpBitmap(const char *filename) const
{
	//for (int i = 0; i < 20; ++i)
	{
		auto color_data = ColorBitmap{ this->size };

		for (int i = 0; i < this->size.y * this->size.x; i++)
            color_data[i] = static_cast<Color>(GetVoxelColor(this->GetVoxelRaw(i)));

		{
			auto trace = MeasureFunction<5>("DumpBitmap");
			BmpFile::SaveToFile(color_data, filename);
		}
	}
}

