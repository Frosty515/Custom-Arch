/*
Copyright (©) 2024  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "SDLVideoBackend.hpp"

#include "IO/devices/Video/VideoBackend.hpp"

void EventHandler(void* data) {
    SDLVideoBackend* backend = static_cast<SDLVideoBackend*>(data);
    backend->EnterEventLoop();
}

SDLVideoBackend::SDLVideoBackend(const VideoMode& mode)
    : VideoBackend(mode), m_window(nullptr), m_renderer(nullptr), m_texture(nullptr), m_framebuffer(nullptr), m_eventThread(nullptr) {
}

SDLVideoBackend::~SDLVideoBackend() {
}

void SDLVideoBackend::Init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Failed to initialise SDL: %s\n", SDL_GetError());
        exit(1);
    }

    VideoMode mode = GetRawMode();

    m_window = SDL_CreateWindow("Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mode.width, mode.height, SDL_WINDOW_SHOWN);
    if (m_window == nullptr) {
        printf("Failed to create window: %s\n", SDL_GetError());
        exit(1);
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (m_renderer == nullptr) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        exit(1);
    }

    m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, mode.width, mode.height);
    if (m_texture == nullptr) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        exit(1);
    }

    m_framebuffer = new uint8_t[mode.width * mode.height * 4];

    memset(m_framebuffer, 0, mode.width * mode.height * 4);

    Draw();

    m_eventThread = new std::thread(EventHandler, this);
}

void SDLVideoBackend::SetMode(VideoMode mode) {
    SetRawMode(mode);

    SDL_DestroyTexture(m_texture);

    m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, mode.width, mode.height);
    if (m_texture == nullptr) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        exit(1);
    }

    delete[] m_framebuffer;
    m_framebuffer = new uint8_t[mode.width * mode.height * 4];

    memset(m_framebuffer, 0, mode.width * mode.height * 4);

    SDL_SetWindowSize(m_window, mode.width, mode.height);

    Draw();
}

VideoMode SDLVideoBackend::GetMode() {
    return GetRawMode();
}

void SDLVideoBackend::Write(uint64_t offset, uint8_t* data, uint64_t size) {
    memcpy(m_framebuffer + offset, data, size);
    Draw();
}

void SDLVideoBackend::Read(uint64_t offset, uint8_t* data, uint64_t size) {
    memcpy(data, m_framebuffer + offset, size);
}

void SDLVideoBackend::EnterEventLoop() {
    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                exit(0);
            default:
                break;
            }
        }
    }
}

void SDLVideoBackend::Draw() {
    VideoMode mode = GetRawMode();

    SDL_UpdateTexture(m_texture, nullptr, m_framebuffer, mode.width * 4);

    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
    SDL_RenderPresent(m_renderer);
}