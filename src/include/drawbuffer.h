#pragma once
#include <memory>
#include <types.h>
#include <colors.h>

class DrawBuffer
{
	std::unique_ptr<Color> pixel_data;
	Size size;
	Color default_color;

public:
	DrawBuffer(Size size);
	Color& DefaultColor() { return default_color; }
	void SetDefaultColor(Color color) { default_color = color; }
	void SetPixel(Position pos, Color color);
	Color GetPixel(Position pos);
};

