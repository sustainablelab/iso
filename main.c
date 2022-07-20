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

void setup_debug_box(TextBox *tb, char *buffer)
{
    tb->text = buffer;                                          // Point at text buffer
    tb->fg = (SDL_Color){255,255,255,255};                      // White text
    tb->bg = (SDL_Color){128,255,0,200};                        // Green glow when active
    tb->margin = 5;                                             // Space (pix) to win edge
    tb->fg_rect=(SDL_Rect){.x=tb->margin, .y=tb->margin,        // Left/Top edge is margin
                           .w=0, .h=0};                         // Will size text in loop
    tb->bg_rect=(SDL_Rect){0};                                  // Will size bgnd in loop
    tb->focus = false;                                          // Start: not user's focus
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
    bool show_debug = true;                                     // Start debug visible
    // Main debug overlay -- print whatever I want here
    TextBox dTB;                                                // Debug overlay Textbox
    char debug_text_buffer[2048];                               // Max 2048 char overlay
    setup_debug_box(&dTB, debug_text_buffer);                   // Init debug overlay
    // Smaller text window on the left just for some controls
    TextBox dCB;                                                // Debug control box
    char debug_controls[128];                                   // Small debug buffer
    char debug_input_buffer[4];                                 // Store at most 4 char
    char *debug_i_end = debug_input_buffer + 4*sizeof(char);
    char *debug_i = debug_input_buffer; *debug_i = '\0';
    setup_debug_box(&dCB, debug_controls);
    // Mode
    enum {
        GAME_MODE,
        DEBUG_WINDOW_MODE,
        DEBUG_INSERT_MODE
        } mode = GAME_MODE;                                     // Be modal

    // Game loop
    while(  quit == false  )
    {
        // Update state
        SDL_GetWindowSize(win, &wI.w, &wI.h);                   // Adapt to window resize
        dTB.bg_rect.w = wI.w - dCB.bg_rect.w;                   // w : main debug text
        switch(mode)
        {
            case GAME_MODE: break;
            case DEBUG_WINDOW_MODE: break;
            default: break;
        }

        // UI
        SDL_Keymod kmod = SDL_GetModState();                    // kmod : OR'd modifiers
        { // Filtered (rapid fire keys)
            SDL_PumpEvents();
            /* const Uint8 *k = SDL_GetKeyboardState(NULL);        // Get all keys */
            /* if(  k[SDL_SCANCODE_ESCAPE]  ) quit = true;         // Esc : quit */
        }
        { // Polled (delayed repeat fire after initial key press)
            SDL_Event e;
            while(  SDL_PollEvent(&e)  )
            {
                if(  e.type == SDL_TEXTINPUT  )
                {
                    if(  mode == DEBUG_INSERT_MODE  )
                    {
                        // TODO: don't insert the `i` that starts insert mode
                        // Copy text
                        const char *c = e.text.text;
                        while(  (*c!='\0') && (debug_i < debug_i_end)  )
                        { *debug_i++ = *c++; }
                        *debug_i = '\0';
                    }
                }
                if(  e.type == SDL_KEYDOWN  )
                {
                    switch(  e.key.keysym.sym  )
                    {
                        case SDLK_ESCAPE:
                            switch(mode)
                            {
                                case GAME_MODE: quit = true; break;
                                case DEBUG_WINDOW_MODE:
                                    mode = GAME_MODE;
                                    dCB.focus = false;
                                    dTB.focus = false;
                                    break;
                                case DEBUG_INSERT_MODE:
                                    mode = DEBUG_WINDOW_MODE;
                                    break;
                                default: quit = true; break;
                            }
                            break;
                        case SDLK_TAB:
                            show_debug = !show_debug;           // Tab: toggle debug vis
                            break;
                        case SDLK_SEMICOLON:
                            if(  kmod & (KMOD_LSHIFT|KMOD_RSHIFT)  )
                            {
                                /* dCB.focus = !dCB.focus;         // : : Debug window mode */
                                if(  mode != DEBUG_WINDOW_MODE  ) dCB.focus = true;
                                mode = DEBUG_WINDOW_MODE;
                            }
                            break;
                        case SDLK_l:
                            if(  mode == DEBUG_WINDOW_MODE  )
                            {
                                if(  dCB.focus  )
                                {
                                    dCB.focus = false;
                                    dTB.focus = true;
                                }
                            }
                            break;
                        case SDLK_h:
                            if(  mode == DEBUG_WINDOW_MODE  )
                            {
                                if(  dTB.focus  )
                                {
                                    dTB.focus = false;
                                    dCB.focus = true;
                                }
                            }
                            break;
                        case SDLK_i:
                            if(  mode == DEBUG_WINDOW_MODE  )
                            {
                                if(  dCB.focus  )
                                {
                                    mode = DEBUG_INSERT_MODE;
                                }
                            }
                            break;
                        default: break;
                    }
                }
            }
        }

        // Render
        { // Background
            SDL_SetRenderDrawColor(ren, 128,128,0,0);
            SDL_RenderClear(ren);
        }
        if(  show_debug  )
        { // Debug overlay
            { // Fill main overlay text buffer with characters
                char *d = dTB.text;                             // d : see macro "print"
                print("This is the main debug overlay.\nStill the main debug overlay.");
            }
            { // Fill debug control text buffer with characters
                char *d = dCB.text;                             // d : see macro "print"
                print("Bgnd\n----\nR: ");print(debug_input_buffer);
            }
            { // Draw the control box text to its texture
                SDL_Surface *surf = TTF_RenderText_Blended_Wrapped(debug_font,
                                        dCB.text,               // Text buffer to render
                                        dCB.fg,                 // Text color
                                        0);                     // Wrap on new lines
                dCB.tex = SDL_CreateTextureFromSurface(ren, surf);
                SDL_FreeSurface(surf);
                SDL_QueryTexture(dCB.tex, NULL, NULL,
                                    &dCB.fg_rect.w,             // Get text width
                                    &dCB.fg_rect.h);            // Get text height
                dCB.bg_rect.h = dCB.fg_rect.h + 2*dCB.margin;   // Extend bgnd below text
                dCB.bg_rect.w = dCB.fg_rect.w + 2*dCB.margin;   // Box around text
            }
            { // Draw the main overlay text to the debug overlay texture
                SDL_Surface *surf = TTF_RenderText_Blended_Wrapped(debug_font,
                                        dTB.text,               // Text buffer to render
                                        dTB.fg,                 // Text color
                        wI.w - dTB.margin - dCB.bg_rect.w);     // Wrap text at this width
                dTB.tex = SDL_CreateTextureFromSurface(ren, surf);
                SDL_FreeSurface(surf);
                SDL_QueryTexture(dTB.tex, NULL, NULL,
                                    &dTB.fg_rect.w,             // Get text width
                                    &dTB.fg_rect.h);            // Get text height
                dTB.fg_rect.x = dTB.margin + dCB.bg_rect.w;     // Scoot text over
                dTB.bg_rect.x = dCB.bg_rect.w;                  // Scoot box over
                dTB.bg_rect.h = dTB.fg_rect.h + 2*dTB.margin;   // Extend bgnd below text
            }
            { // Render : bgnd fill-rect, then copy debug overlay texture on top
                SDL_SetRenderDrawColor(ren,0,0,0,127);          // Blk bgnd, 50% opacity
                int tallest;
                { // Get height of tallest text
                    int t = dTB.fg_rect.h; int c = dCB.fg_rect.h;
                    tallest = (t > c) ? t : c;
                }
                SDL_Rect bg_rect = {.x=0, .y=0, .w=wI.w, .h=(tallest + 2*dTB.margin)};
                SDL_RenderFillRect(ren, &bg_rect);              // Draw black bgnd
                if(  dCB.focus  )                               // Glow if in user's focus
                {
                    SDL_SetRenderDrawColor(ren, dCB.bg.r, dCB.bg.g, dCB.bg.b, dCB.bg.a);
                    SDL_RenderDrawRect(ren, &dCB.bg_rect);      // Draw green glow box
                }
                else if(  dTB.focus  )
                {
                    SDL_SetRenderDrawColor(ren, dTB.bg.r, dTB.bg.g, dTB.bg.b, dTB.bg.a);
                    SDL_RenderDrawRect(ren, &dTB.bg_rect);      // Draw green glow box
                }
                SDL_RenderCopy(ren, dTB.tex, NULL, &dTB.fg_rect);
                SDL_RenderCopy(ren, dCB.tex, NULL, &dCB.fg_rect);
                SDL_DestroyTexture(dTB.tex);
                SDL_DestroyTexture(dCB.tex);
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
