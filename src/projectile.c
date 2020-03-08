#include <cstdio>
#include <cstdlib>

#include <projectile.h>
#include <level.h>
#include <random.h>
#include <tank.h>
#include <tweak.h>
#include <memalloc.h>
#include <drawbuffer.h>
#include <tanklist.h>


Projectile::Projectile(Position position, Position origin, Speed speed, int life, bool is_effect, Tank* tank)
    : pos(position), pos_old(origin), step(speed), life(life), is_effect(is_effect), tank(tank), is_alive(true)
{

}

std::vector<Projectile> Projectile::CreateExplosion(Position pos, int count, int r, int ttl)
{
	auto items = std::vector<Projectile>{};
	items.reserve(count);
	/* Add all of the effect particles: */
	for (int i = 0; i < count; i++) {
		items.emplace_back(Projectile{
			Position{pos.x * 16 + 8, pos.y * 16 + 8},
			Position{pos.x, pos.y},
			Speed { Random::Int(0,r) - r / 2, Random::Int(0,r) - r / 2},
			Random::Int(0,ttl), 1, nullptr });
	}
	return items;
}

Projectile Projectile::CreateBullet(Tank* t)
{
	int dir = t->GetDirection();
	Speed speed = Speed{ static_cast<int>(dir) % 3 - 1, static_cast<int>(dir) / 3 - 1 };
	return Projectile {
	    t->GetPosition(),
	    t->GetPosition(),
	    speed,
	    TANK_BULLET_SPEED, 0, t };
}

Projectile& ProjectileList::Add(Projectile projectile)
{
	return this->container.Add(projectile);
}

void ProjectileList::Add(std::vector<Projectile> projectiles)
{
	for(auto& projectile : projectiles)
		this->container.Add(projectile);
}

void ProjectileList::Advance(Level* level, TankList* tankList)
{
	auto newly_added = std::vector<Projectile>{};
	/* Iterate over the living objects: */
	for (Projectile& p : container)
	{
		/* Is this a bullet, or an effect? */
		if (p.is_effect) {
			int x, y;

			/* Effect: */

			/* Did this expire? */
			if (!p.life) {
				Remove(p);
				continue;
			}

			/* Move the effect: */
			p.life--;
			p.pos.x += p.step.x; p.pos.y += p.step.y;
			x = p.pos.x / 16; y = p.pos.y / 16;

			/* Make sure we didn't hit a level detail: */
			char c = level_get(level, x, y);
			if (c != DIRT_HI && c != DIRT_LO && c != BLANK) {
				Remove(p);
				continue;
			}

			/* Effects blank everything out in their paths: */
			level_set(level, x, y, BLANK);

		}
		else {
			int i;

			/* Bullet: */

			for (i = 0; i < p.life; i++) {
				Tank* t;
				int clr;

				p.pos_old.x = p.pos.x;     p.pos_old.y = p.pos.y;
				p.pos.x += p.step.x;		 p.pos.y += p.step.y;

				/* Did we hit another tank? */
				clr = p.tank->GetColor();
				t = tankList->GetTankAtPoint(p.pos.x, p.pos.y, clr);
				if (t) {
					/* If we have an associated tank, return the shot: */
					p.tank->ReturnBullet();

					/* Hurt the tank we hit: */
					t->AlterHealth(TANK_SHOT_DAMAGE);

					/* Add all of the effect particles: */

					newly_added.reserve(newly_added.size() + EXPLOSION_HURT_COUNT);
					for (Projectile& proj : Projectile::CreateExplosion(Position{ p.pos_old.x, p.pos_old.y },
						EXPLOSION_HURT_COUNT,
						EXPLOSION_HURT_RADIUS,
						EXPLOSION_HURT_TTL))
					{
						newly_added.emplace_back(proj);
					}

					/* Finally, remove it: */
					Remove(p);
					break;
				}

				/* Else, did we hit something in the level? */
				char c = level_get(level, p.pos.x, p.pos.y);
				if (c != BLANK) {
					/* If we have an associated tank, return the shot: */
					p.tank->ReturnBullet();

					/* Add all of the effect particles: */
					newly_added.reserve(newly_added.size() + EXPLOSION_DIRT_COUNT);
					for (Projectile& proj : Projectile::CreateExplosion(Position{ p.pos_old.x, p.pos_old.y },
						EXPLOSION_DIRT_COUNT,
						EXPLOSION_DIRT_RADIUS,
						EXPLOSION_DIRT_TTL))
					{
						newly_added.emplace_back(proj);
					}

					/* Finally, remove it: */
					Remove(p);
					break;
				}
			}
		}
	}

	Add(newly_added);
	Shrink();
}

void ProjectileList::Erase(DrawBuffer* drawBuffer)
{
	for (Projectile& p : container)
	{
		if (p.is_effect)
			drawbuffer_set_pixel(drawBuffer, p.pos.x / 16, p.pos.y / 16, color_blank);
		else {
			drawbuffer_set_pixel(drawBuffer, p.pos.x, p.pos.y, color_blank);
			drawbuffer_set_pixel(drawBuffer, p.pos_old.x, p.pos_old.y, color_blank);
		}
	}
}


void ProjectileList::Draw(DrawBuffer* drawBuffer)
{
	for(Projectile& p : container)
	{
		if (p.is_effect)
			drawbuffer_set_pixel(drawBuffer, p.pos.x / 16, p.pos.y / 16, color_fire_hot);
		else {
			drawbuffer_set_pixel(drawBuffer, p.pos.x, p.pos.y, color_fire_hot);
			drawbuffer_set_pixel(drawBuffer, p.pos_old.x, p.pos_old.y, color_fire_cold);
		}
	}
}
