#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "window_info.h"
#include "font.h"

// Singletons
SDL_Window *win;                                                // The window
SDL_Renderer *ren;                                              // The renderer
TTF_Font *debug_font;                                           // Debug overlay font

void shutdown(void)
{
    TTF_CloseFont(debug_font);
    TTF_Quit();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

int font_setup(TTF_Font **font)
{
    if(  font_init() < 0  ) { shutdown(); return -1; }          // Init SDL_ttf
    // Font p : path and s : size
    const char *p = "fonts/ProggyClean.ttf";                    // Path to debug font
    int s = 16;                                                 // Font point size
    if(  font_load(font, p, s) < 0  ) { shutdown(); return -1;} // Load the debug font
    return 0;
}

int main(int argc, char *argv[])
{
    // Setup
    for(int i=0; i<argc; i++) puts(argv[i]);                    // List cmdline args
    SDL_Init(SDL_INIT_VIDEO);                                   // Init SDL
    WindowInfo wI; WindowInfo_setup(&wI, argc, argv);           // Init game window info
    win = SDL_CreateWindow(
            argv[0], wI.x, wI.y, wI.w, wI.h, wI.flags);         // Create the window
    ren = SDL_CreateRenderer(win, -1, 0);                       // Create the renderer
    if(  font_setup(&debug_font) < 0  ) return EXIT_FAILURE;    // Init TTF and load font

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
    shutdown();
    return EXIT_SUCCESS;
}
