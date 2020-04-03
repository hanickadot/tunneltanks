﻿#include "sdl_system.h"

#include "game_config.h"

SdlSystem::SdlSystem(VideoConfig video_config)
    : window(video_config.resolution, video_config.is_fullscreen),
      renderer(&this->window, video_config.render_surface_size), cursor()
{
    window.AttachRenderer(&this->renderer);
}

std::unique_ptr<GameSystem> CreateGameSystem(VideoConfig video_config)
{
    return std::make_unique<SdlSystem>(video_config);
}
