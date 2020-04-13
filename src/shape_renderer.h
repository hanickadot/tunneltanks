﻿#pragma once

#include <optional>

#include "containers.h"
#include "mymath.h"
#include "random.h"
#include "raycaster.h"
#include "render_surface.h"
#include "types.h"

struct Color;
class Screen;

class ShapeRenderer
{
  public:
    static void FillRectangle(Surface * surface, Rect rect, Color color);
    static void DrawRectangle(Surface * screen, Rect rect, bool round_corners, Color fill_color,
                              Color outline_color);
     static void DrawCircle(Surface * screen, Position center, int radius, Color fill_color, Color outline_color);
};

class ShapeInspector
{
  public:
    static Position GetRandomPointInCircle(Position center, int radius);
    template <typename InspectFunc>
    static std::optional<Position> FromRandomPointInCircleToCenter(Position center, int radius,
                                                                   InspectFunc inspect_func);
    template <typename SurfaceType, typename InspectFunc>
    static bool InspectRectangle(const SurfaceType & container, Rect rect, InspectFunc inspect_func);

};

/* InspectFunc (Position position) -> bool
 *  Return false to continue inspection, true to terminate */
template <typename InspectFunc>
std::optional<Position> ShapeInspector::FromRandomPointInCircleToCenter(Position center, int radius,
                                                                        InspectFunc inspect_func)
{
    Position possible_pos = ShapeInspector::GetRandomPointInCircle(center, radius);
    if (inspect_func(possible_pos))
        return possible_pos;

    /* If found not suitable, cast a ray to the center and find first pixel that does */
    if (Raycaster::Cast(
            PositionF{possible_pos}, PositionF{center},
            [&possible_pos, inspect_func](PositionF tested_pos, PositionF previous_pos) {
                if (inspect_func(tested_pos.ToIntPosition()))
                {
                    possible_pos = tested_pos.ToIntPosition();
                    return false;
                }
                return true;
            },
            Raycaster::VisitFlags::PixelsMustTouchCorners))
        return possible_pos;
    return std::nullopt;
}

/*
 * Inspects every item laying on (not inside) given rectangle.
 *   return true if every inspection returned true as well, false otherwise
 *   stops on first failed inspection.
 * SurfaceType - a 2D container indexed by Position
 * InspectFunc - return true to continue inspection, false to terminate */
template <typename SurfaceType, typename InspectFunc>
bool ShapeInspector::InspectRectangle(const SurfaceType & container, Rect rect, InspectFunc inspect_func)
{
    for (int x = rect.Left(); x <= rect.Right(); ++x)
        for (int y = rect.Top(); y <= rect.Bottom(); ++y)
        {
            /* Are we inside edges?  */
            if (x != rect.Left() && x != rect.Right() && y != rect.Top() && y != rect.Bottom())
                continue;
            Position pos = {x, y};
            if (!inspect_func(pos, container[pos]))
                return false;
        }
    return true;
}

