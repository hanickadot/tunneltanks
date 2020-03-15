#include "base.h"
#include <screen.h>
#include <tweak.h>
#include <random.h>
#include <level.h>
#include <tank.h>
#include <types.h>
#include <drawbuffer.h>
#include <gamelib.h>
#include "colors.h"

/* The constructor sets the video mode: */
Screen::Screen(bool is_fullscreen)
	: is_fullscreen(is_fullscreen)
{
	this->Resize({ tweak::screen::size.x, tweak::screen::size.y });
}


/* Fills a surface with a blue/black pattern: */
void Screen::FillBackground() {
	Size dim = gamelib_get_resolution();
	
	gamelib_draw_box({{ 0, 0}, dim}, Palette.Get(Colors::Background));
	Position o;
	for(o.y = 0; o.y<dim.y; o.y++) {
		for(o.x = (o.y%2)*2; o.x<dim.x; o.x+=4) {
			gamelib_draw_box(Rect{ o, Size {1,1} }, Palette.Get(Colors::BackgroundDot));
		}
	}
}

void Screen::DrawPixel(ScreenPosition pos, Color color) {

	Offset adjusted_size = {  /* Make some pixels uniformly larger to fill in given space relatively evenly  */
		(pos.x * this->pixels_skip.x) / GAME_WIDTH,
		(pos.y * this->pixels_skip.y) / GAME_HEIGHT };
	Offset adjusted_next = {
		((pos.x + 1) * this->pixels_skip.x) / GAME_WIDTH,
		((pos.y + 1) * this->pixels_skip.y) / GAME_HEIGHT };
	
	/* Final pixel position, adjusted by required scaling and offset */ 
	pos.x = (pos.x * this->pixel_size.x) + this->screen_offset.x + adjusted_size.x;
	pos.y = (pos.y * this->pixel_size.y) + this->screen_offset.y + adjusted_size.y;
	
	auto pixelSize = Size { /* Compute size based on needing uneven scaling or not */
		this->pixel_size.x + (adjusted_size.x != adjusted_next.x),
		this->pixel_size.y + (adjusted_size.y != adjusted_next.y)
	};
	
	gamelib_draw_box(Rect{ static_cast<Position>(pos), pixelSize }, color);
}


Position Screen::ScreenToWorld(ScreenPosition screen_pos) {

	Position pos = (Position)screen_pos;
	pos.x -= this->screen_offset.x;
	pos.x -= pos.x/(int)this->pixel_size.x * (int)this->pixels_skip.x / GAME_WIDTH;
	pos.x /= (int)this->pixel_size.x;

	pos.y -= this->screen_offset.y;
	pos.y -= pos.y / (int)this->pixel_size.y * (int)this->pixels_skip.y / GAME_HEIGHT;
	pos.y /= (int)this->pixel_size.y;
	
	return pos;
}

void Screen::DrawLevel() {

	std::for_each(this->windows.begin(), this->windows.end(), [this](auto& item) { item.Draw(this); });
	std::for_each(this->statuses.begin(), this->statuses.end(), [this](auto& item) { item.Draw(this); });
	std::for_each(this->bitmaps.begin(), this->bitmaps.end(), [this](auto& item) { item.Draw(this); });

	/*if(this->controller_count)
		gamelib_gui_draw(this, this->controller.r);*/
}

void Screen::DrawCurrentMode() {	
	if(this->mode == SCREEN_DRAW_LEVEL) {
		this->DrawLevel();
	}
	//throw GameException("Invalid mode to draw");
}


/* TODO: Change the screen API to better match gamelib... */

void Screen::SetFullscreen(bool new_fullscreen) {
	
	if(this->is_fullscreen == new_fullscreen) return;
	
	this->is_fullscreen = new_fullscreen;
	
	/* Resize the screen to include the new fullscreen mode: */
	if (!is_fullscreen) this->Resize(Size{ tweak::screen::size.x, tweak::screen::size.y });
	else                this->Resize(this->screen_size);
}


/* Returns if successful */
void Screen::Resize(Size size)
{
	Size render_size;
	this->pixels_skip = {};
	this->screen_offset = {};
	this->pixel_size = {};

	/* Make sure that we aren't scaling to something too small: */
	size.x = std::max(GAME_WIDTH, size.x);
	size.y = std::max(GAME_HEIGHT, size.y);
	
	/* A little extra logic for fullscreen: */
	if(this->is_fullscreen) gamelib_set_fullscreen();
	else                    gamelib_set_window    (size);
	
	size = gamelib_get_resolution();
	
	this->is_fullscreen = gamelib_get_fullscreen();
	
	/* What is the limiting factor in our scaling to maintain aspect ratio? */
	int yw = size.y * GAME_WIDTH; 
	int xh = size.x * GAME_HEIGHT;
	if( yw < xh ) {
		/* size.y is. Correct aspect ratio using offset */
		render_size.x = (GAME_WIDTH * size.y) / (GAME_HEIGHT);
		render_size.y = size.y;
		this->screen_offset.x = (size.x - render_size.x)/2;
		this->screen_offset.y = 0;
	} else {
		/* size.x is. Correct aspect ratio using offset */
		render_size.x = size.x;
		render_size.y = (GAME_HEIGHT * size.x) / (GAME_WIDTH);
		this->screen_offset.x = 0;
		this->screen_offset.y = (size.y - render_size.y)/2;
	}
	
	/* Calculate the pixel sizing variables: */
	this->pixel_size.x = render_size.x / GAME_WIDTH;
	this->pixel_size.y = render_size.y / GAME_HEIGHT;
	this->pixels_skip.x = render_size.x % GAME_WIDTH;
	this->pixels_skip.y = render_size.y % GAME_HEIGHT;
	
	/* Draw a nice bg: */
	Screen::FillBackground();
	
	this->screen_size = size;
	
	/* Redraw the game: */
	this->DrawCurrentMode();
}

/* Set the current drawing mode: */
void Screen::SetLevelDrawMode(DrawBuffer *b) {
	this->mode = SCREEN_DRAW_LEVEL;
	this->drawBuffer = b;
}

/*
void Screen::set_mode_menu( Menu *m) ;
void Screen::set_mode_map( Map *m) ;
*/

/* Window creation should only happen in Level-drawing mode: */
void Screen::AddWindow( Rect rect, Tank *task) {
	if(this->mode != SCREEN_DRAW_LEVEL) return;
	this->windows.emplace_back(widgets::TankView{ rect, task});
}

/* We can add the health/energy status bars here: */
void Screen::AddStatus(Rect rect, Tank *tank, bool decreases_to_left) {
	/* Verify that we're in the right mode, and that we have room: */
	if(this->mode != SCREEN_DRAW_LEVEL) return;

	/* Make sure that this status bar isn't too small: */
	if(rect.size.x <= 2 || rect.size.y <= 4) return;
	this->statuses.emplace_back(widgets::StatusBar{ rect, tank, decreases_to_left });
}

/* We tell the graphics system about GUI graphics here: 
 * 'color' has to be an ADDRESS of a color, so it can monitor changes to the
 * value, especially if the bit depth is changed... 
 * TODO: That really isn't needed anymore, since we haven't cached mapped RGB
 *       values since the switch to gamelib... */
void Screen::AddBitmap( Rect rect, Bitmap* new_bitmap, Color color) {
	/* Bitmaps are only for game mode: */
	if(this->mode != SCREEN_DRAW_LEVEL) return;
	if(!new_bitmap) return;
	this->bitmaps.emplace_back(widgets::BitmapRender{ rect, new_bitmap, color });
}

void Screen::ClearGuiElements()
{
	this->bitmaps.clear();
	this->windows.clear();
	this->statuses.clear();
}

/* We don't check to see if gamelib needs the gui controller thing in this file.
 * That is handled in game.c: */
void Screen::AddController( Rect r) {
	if(this->mode != SCREEN_DRAW_LEVEL) return;
		this->controller.r = r;
}

