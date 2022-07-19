#include <SDL.h>
#include <stdbool.h>
#include "window_info.h"


void shutdown(SDL_Renderer *ren, SDL_Window *win)
{
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    // Setup
    for(int i=0; i<argc; i++) puts(argv[i]);                    // List cmdline args
    SDL_Init(SDL_INIT_VIDEO);                                   // Init SDL
    WindowInfo wI; WindowInfo_setup(&wI, argc, argv);           // Init game window info
    SDL_Window *win = SDL_CreateWindow(                         // Create the window
            argv[0], wI.x, wI.y, wI.w, wI.h, wI.flags
            );
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, 0);         // Create the renderer

    // Game state
    bool quit = false;

    // Game loop
    while(  quit == false  )
    {
        // Update state

        // UI
        { // Filtered (rapid fire keys)
            SDL_PumpEvents();
            const Uint8 *k = SDL_GetKeyboardState(NULL);            // Get all keys
            if(  k[SDL_SCANCODE_ESCAPE]  ) quit = true;             // Esc : quit
        }

        // Render
        { // Background
            SDL_SetRenderDrawColor(ren, 0,0,0,0);
            SDL_RenderClear(ren);
        }
        { // Present to screen
            SDL_RenderPresent(ren);
            SDL_Delay(10);
        }
    }

    // Shutdown
    shutdown(ren, win);
    return EXIT_SUCCESS;
}
