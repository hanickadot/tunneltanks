#pragma once
#include <level.h>
#include <types.h>
#include <tank.h>
#include <drawbuffer.h>


typedef enum ScreenDrawMode {
	SCREEN_DRAW_INVALID,
	SCREEN_DRAW_LEVEL
} ScreenDrawMode;

typedef struct Window {
	Rect  r;
	class Tank* t;
	int counter;
	int showing_static;
} Window;

typedef struct StatusBar {
	Rect  r;
	class Tank* t;
	int      decreases_to_left;
} StatusBar;

typedef struct Bitmap {
	Rect     r;
	char* data;
	Color color;
} Bitmap;

typedef struct GUIController {
	Rect r;
} GUIController;


class Screen {

	bool	 is_fullscreen;

	/* Various variables for the current resolution: */
	Size screen_size = {};
	Offset screen_offset = {};
	Size pixel_size = {};
	Size pixels_skip = {};

	std::vector<Window> windows;
	std::vector<StatusBar> statuses;
	std::vector<Bitmap> bitmaps;

	/* GUI Controller shit: */
	int controller_count = 0;
	GUIController controller;
	/* Variables used for drawing: */
	ScreenDrawMode mode = SCREEN_DRAW_INVALID;
	DrawBuffer* drawBuffer = nullptr;
public:
	Screen(bool is_fullscreen);

	/* Resizing the screen: */
	bool GetFullscreen() { return is_fullscreen; }
	void SetFullscreen(bool is_fullscreen);
	void Resize(Size size);

	/* Set the current drawing mode: */
	void SetLevelDrawMode(DrawBuffer* b);
	/* Will randomly draw static to a window, based on a tank's health.  */
	void DrawStatic(Window* w);
	/* Will draw a window using the level's drawbuffer: */
	void DrawWindow(Window* w);

	/* A few useful functions for external drawing: */
	void DrawPixel(ScreenPosition pos, Color color);
	/* These will say what virtual pixel a physical pixel resides on: */
	Position ScreenToWorld(ScreenPosition pos);
	/*  Draw whatever it is now supposed to draw */
	void DrawCurrentMode();

	/* Will draw two bars indicating the charge/health of a tank: */
	void DrawStatus(StatusBar* b);
	void DrawBitmap(Bitmap* b);
	void DrawLevel();
public:
	void AddWindow(Rect r, class Tank* t);
	void AddStatus(Rect r, class Tank* t, int decreases_to_left);
	void AddBitmap(Rect r, char* bitmap, Color color);
	/* Call to remove all windows, statuses and bitmaps */
	void ClearGuiElements();
	
	void AddController(Rect r);

	static void FillBackground();
};
