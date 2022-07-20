#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "window_info.h"
#include "font.h"
#include "text_box.h"
#include "print.h"

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

void setup_debug_overlay(TextBox *tb, char *buffer)
{
    tb->text = buffer;                                          // Point at text buffer
    tb->fg = (SDL_Color){255,255,255,255};                      // White text
    tb->bg = (SDL_Color){0,0,0,127};                            // Blk bgnd, 50% opacity
    tb->margin = 5;                                             // Space (pix) to win edge
    tb->fg_rect=(SDL_Rect){.x=tb->margin, .y=tb->margin,        // Left/Top edge is margin
                           .w=0, .h=0};                         // Will size text in loop
    tb->bg_rect=(SDL_Rect){0};                                  // Will size bgnd in loop
    tb->show = true;                                            // Start: overlay visible
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
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);       // Draw with alpha
    if(  font_setup(&debug_font) < 0  ) return EXIT_FAILURE;    // Init TTF and load font

    // Game state
    bool quit = false;
    TextBox dTB;                                                // Debug overlay Textbox
    char debug_text_buffer[2048];                               // Max 2048 char overlay
    setup_debug_overlay(&dTB, debug_text_buffer);               // Init debug overlay

    // Game loop
    while(  quit == false  )
    {
        // Update state
        SDL_GetWindowSize(win, &wI.w, &wI.h);                   // Adapt to window resize
        dTB.bg_rect.w = wI.w;                                   // Debug bgnd: full width

        // UI
        { // Filtered (rapid fire keys)
            SDL_PumpEvents();
            const Uint8 *k = SDL_GetKeyboardState(NULL);        // Get all keys
            if(  k[SDL_SCANCODE_ESCAPE]  ) quit = true;         // Esc : quit
        }
        { // Polled (delayed repeat fire after initial key press)
            SDL_Event e;
            while(  SDL_PollEvent(&e)  )
            {
                if(  e.type == SDL_KEYDOWN  )
                {
                    switch(  e.key.keysym.sym  )
                    {
                        case SDLK_TAB: dTB.show = !dTB.show;    // Tab: toggle debug vis
                    }
                }
            }
        }

        // Render
        { // Background
            SDL_SetRenderDrawColor(ren, 128,128,0,0);
            SDL_RenderClear(ren);
        }
        if(  dTB.show  )
        { // Debug overlay
            { // Fill text buffer with characters
                char *d = dTB.text;                             // d : see macro "print"
                print("I be some text");
            }
            { // Draw the text to the debug overlay texture
                SDL_Surface *surf = TTF_RenderText_Blended_Wrapped(debug_font,
                                        dTB.text,               // Text buffer to render
                                        dTB.fg,                 // Text color
                                        wI.w - dTB.margin);     // Wrap text here
                dTB.tex = SDL_CreateTextureFromSurface(ren, surf);
                SDL_FreeSurface(surf);
                SDL_QueryTexture(dTB.tex, NULL, NULL,
                                    &dTB.fg_rect.w,             // Get text width
                                    &dTB.fg_rect.h);            // Get text height
                dTB.bg_rect.h = dTB.fg_rect.h + 2*dTB.margin;   // Extend bgnd below text
            }
            { // Render : bgnd fill-rect, then copy debug overlay texture on top
                SDL_SetRenderDrawColor(ren, dTB.bg.r, dTB.bg.g, dTB.bg.b, dTB.bg.a);
                SDL_RenderFillRect(ren, &dTB.bg_rect);
                SDL_RenderCopy(ren, dTB.tex, NULL, &dTB.fg_rect);
                SDL_DestroyTexture(dTB.tex);
            }
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
