#include "fuji.h"
#include "fmt/core.h"
#include "SDL.h"
#include <SDL_video.h>



int fuji(){
    #ifdef NDEBUG
    fmt::print("fuji/0.1: Release Build\n");
    #else
    fmt::print("fuji/0.1: Debug Build\n");
    #endif

    SDL_Window* window = SDL_CreateWindow(
        "fuji", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        800, 
        600, 
        SDL_WindowFlags::SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (!window) {
        fmt::print("Error creating window");
        return EXIT_FAILURE;
    }

    SDL_Event* event;
    while (SDL_PollEvent(event)) {
        if (!event)
            continue;

        if (event->type == SDL_QUIT)
            break;
    }

    return 0;
}
