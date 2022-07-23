/* *************TODO***************
 * ~1. Add green and blue inputs~
 * ~2. How do I detect which one I'm editing? I'm making each its own box with an SOA~
 * 1. Show initial values of inputs when program starts
 * 2. Improve text-editing interface (see TODO)
 * 3. Add a cursor and give it navigation inside the text
 * 4. Isometric mapping!
 * *******************************/
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "window_info.h"
#include "print.h"
#include "font.h"
#include "debug_text.h"
#include "text_box.h"
#include "help.h"
#include "controls.h"


// Singletons
SDL_Window *win;                                                // The window
SDL_Renderer *ren;                                              // The renderer
TTF_Font *debug_font;                                           // Debug overlay font
Ctrl_SOA *cS;                                                   // Debug controls are SOA


void shutdown(void)
{
    for (int i=0; i<NUM_CTRLS; i++)
    { // Free all string buffers for control text inputs
        free(cS->text[i]);
        cS->text[i] = NULL;
        free(cS->buff_in[i]);
        cS->buff_in[i] = NULL;
        cS->buff_c[i] = NULL;
        cS->buff_end[i] = NULL;
    }
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
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);       // Draw with alpha
    if(  font_setup(&debug_font) < 0  ) return EXIT_FAILURE;    // Init TTF and load font

    // Game state
    bool quit = false;
    bool show_debug = true;                                     // Start debug visible
    Ctrl_SOA controls_in_debug_overlay;                         // Allocate struct mem
    cS = &controls_in_debug_overlay;                            // Point global at struct
    ctrl_load_table(cS);
    for (int i=0; i<NUM_CTRLS; i++)
    { // Create string buffers for control text inputs
        cS->text[i] = malloc(sizeof(char)*8);                   // Entire ctrl: at most 8
        int len = 4;
        cS->buff_in[i] = malloc(sizeof(char)*len);              // Input: at most 4 char
        cS->buff_c[i] = cS->buff_in[i];                         // Point at buff start
        cS->buff_end[i] = cS->buff_in[i] + (len-1)*sizeof(char);// Point at buff end
        *(cS->buff_c[i]) = '\0';                                // nul-terminate start
    }
    for (int i=0; i<NUM_CTRLS; i++)
    { // Initialize text fgnd and bgnd rectangles to zero
        cS->fg_rect[i].x = dT_margin;
        cS->fg_rect[i].y = dT_margin;
        cS->fg_rect[i].w = 0;
        cS->fg_rect[i].h = 0;
        cS->bg_rect[i].x = 0;
        cS->bg_rect[i].y = 0;
        cS->bg_rect[i].w = 0;
        cS->bg_rect[i].h = 0;
    }

    // Main debug overlay -- print whatever I want here
    TextBox dTB;                                                // Debug overlay Textbox
    char debug_text_buffer[2048];                               // Max 2048 char overlay
    setup_debug_box(&dTB, debug_text_buffer);                   // Init debug overlay
    // Smaller text window on the left to title the controls
    TextBox dCB;                                                // Debug control box
    char debug_controls[20];                                    // Small debug buffer
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
                        for( int i=0; i<NUM_CTRLS; i++ )
                        {
                            if(  cS->focus[i]  )
                            {
                                while(  (*c!='\0') && (cS->buff_c[i] < cS->buff_end[i])  )
                                { *(cS->buff_c[i])++ = *c++; }
                                if(  cS->buff_c[i] > cS->buff_end[i]  ) cS->buff_c[i] = cS->buff_end[i];
                                *cS->buff_c[i] = '\0';                  // nul-terminate string
                                break;
                            }
                        }
                    }
                }
                if(  e.type == SDL_KEYDOWN  )
                {
                    switch(  e.key.keysym.sym  )
                    {
                        case SDLK_BACKSPACE:
                            if(  mode == DEBUG_INSERT_MODE  )
                            {
                                for( int i=0; i<NUM_CTRLS; i++ )
                                {
                                    if(  cS->focus[i]  )
                                    {
                                        cS->buff_c[i]--;
                                        if(  cS->buff_c[i] < cS->buff_in[i]  ) cS->buff_c[i] = cS->buff_in[i];
                                        *(cS->buff_c[i]) = '\0';        // nul-terminate string
                                        break;
                                    }
                                }
                            }
                            break;
                        case SDLK_RETURN:
                            if(  mode == DEBUG_INSERT_MODE  )
                            {
                                for( int i=0; i<NUM_CTRLS; i++ )
                                {
                                    if(  cS->focus[i]  )
                                    {
                                        // TODO: do something to text vis to indicate "submitted"
                                        cS->val[i] = atoi(cS->buff_in[i]);
                                        break;
                                    }
                                }
                                // TODO: should return take you out of insert mode or not?
                                /* mode = DEBUG_WINDOW_MODE;       // Done entering text */
                                if(  kmod & (KMOD_LSHIFT|KMOD_RSHIFT)  )
                                {
                                    for( int i=0; i<NUM_CTRLS-1; i++ )
                                    {
                                        if(  cS->focus[i]  )
                                        {
                                            cS->focus[i] = false;
                                            cS->focus[i+1] = true;
                                            break;
                                        }
                                    }

                                }
                            }
                            break;
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
                                if(  mode != DEBUG_WINDOW_MODE  ) dCB.focus = true;
                                mode = DEBUG_WINDOW_MODE;
                            }
                            break;
                        case SDLK_l:
                            if(  mode == DEBUG_WINDOW_MODE  )
                            {
                                bool *TheFocus = NULL;           // Who is in focus?
                                if(  dCB.focus  ) TheFocus = &(dCB.focus);
                                else ctrl_has_focus(&TheFocus, cS); // Search the controls
                                if(  TheFocus != NULL  )
                                { // Found a control with focus
                                    *TheFocus = false;           // Now they are not in focus
                                    dTB.focus = true;           // Now this guy is in focus
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
                        case SDLK_j:
                            if(  mode == DEBUG_WINDOW_MODE  )
                            {
                                if(  dCB.focus  )
                                {
                                    dCB.focus = false;
                                    cS->focus[0] = true;
                                }
                                else if(  !(dCB.focus  || dTB.focus)  )
                                {
                                    for(int i=0; i<(NUM_CTRLS-1); i++)
                                    {
                                        if(  cS->focus[i]  )
                                        {
                                            cS->focus[i] = false;
                                            cS->focus[i+1] = true;
                                            break;
                                        }
                                    }
                                }
                            }
                            break;
                        case SDLK_k:
                            if(  mode == DEBUG_WINDOW_MODE  )
                            {
                                if(  cS->focus[0]  )
                                {
                                    cS->focus[0] = false;
                                    dCB.focus = true;
                                }
                                else
                                {
                                    for( int i=1; i<NUM_CTRLS; i++ )
                                    {
                                        if(  cS->focus[i]  )
                                        {
                                            cS->focus[i] = false;
                                            cS->focus[i-1] = true;
                                            break;
                                        }
                                    }
                                }
                            }
                            break;
                        case SDLK_i:
                            if(  mode == DEBUG_WINDOW_MODE  )
                            {
                                for( int i=0; i<NUM_CTRLS; i++)
                                {
                                    if(  cS->focus[i]  )
                                    {
                                        mode = DEBUG_INSERT_MODE;
                                        break;
                                    }
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
            SDL_SetRenderDrawColor(ren, cS->val[R],cS->val[G],cS->val[B],cS->val[A]);
            SDL_RenderClear(ren);
        }
        if(  show_debug  )
        { // Debug overlay
            { // Fill main overlay text buffer with characters
                char *d = dTB.text;                             // d : see macro "print"
                print(main_overlay_text);
            }
            { // Fill debug control title text buffer with characters
                char *d = dCB.text;                             // d : see macro "print"
                print("Control\n");
                print("-------\n");
            }
            for(int i=0; i<NUM_CTRLS; i++)
            { // Fill control text buffers with characters
                char *d = cS->text[i];                          // d : see macro "print"
                print(cS->label[i]); print(cS->buff_in[i]);
            }
            { // Draw the control title text to its texture
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
            for(int i=0; i<NUM_CTRLS; i++)
            { // Draw the control text inputs to their textures
                SDL_Surface *surf = TTF_RenderText_Blended_Wrapped(
                        debug_font,
                        cS->text[i],                            // Text buffer to render
                        dT_color,                               // Text color
                        0);                                     // Wrap on new lines
                cS->tex[i] = SDL_CreateTextureFromSurface(ren, surf);
                SDL_FreeSurface(surf);
                SDL_QueryTexture(cS->tex[i], NULL, NULL,
                                    &(cS->fg_rect[i].w),        // Get text width
                                    &(cS->fg_rect[i].h));       // Get text height
                // Slide text down based on index
                cS->fg_rect[i].y = dT_margin + dCB.bg_rect.h + i*(cS->fg_rect[i].h);
                cS->bg_rect[i].y = cS->fg_rect[i].y;
                cS->bg_rect[i].h = cS->fg_rect[i].h;            // Fit text height
                cS->bg_rect[i].w = dCB.bg_rect.w;               // Match title width
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
                    int t = dTB.fg_rect.h;
                    /* int c = dCB.fg_rect.h; */
                    int c = dCB.fg_rect.h + 4*(cS->fg_rect[R].h);
                    tallest = (t > c) ? t : c;
                }
                SDL_Rect bg_rect = {.x=0, .y=0, .w=wI.w, .h=(tallest + 2*dTB.margin)};
                SDL_RenderFillRect(ren, &bg_rect);              // Draw black bgnd
                for( int i=0; i<NUM_CTRLS; i++)
                {
                    if(  cS->focus[i]  )                        // Glow if in user's focus
                    {
                        SDL_SetRenderDrawColor(ren, dT_glow.r, dT_glow.g, dT_glow.b, dT_glow.a);
                        SDL_RenderDrawRect(ren, &cS->bg_rect[i]);      // Draw green glow box
                        SDL_SetRenderDrawColor(ren, dT_glow.r, dT_glow.g, dT_glow.b, 100);
                        SDL_RenderFillRect(ren, &cS->bg_rect[i]);      // Fill green glow box
                    }
                }
                if(  dCB.focus  )                               // Glow if in user's focus
                {
                    SDL_SetRenderDrawColor(ren, dCB.bg.r, dCB.bg.g, dCB.bg.b, dCB.bg.a);
                    SDL_RenderDrawRect(ren, &dCB.bg_rect);      // Draw green glow box
                    SDL_SetRenderDrawColor(ren, dCB.bg.r, dCB.bg.g, dCB.bg.b, 100);
                    SDL_RenderFillRect(ren, &dCB.bg_rect);      // Fill green glow box
                }
                else if(  dTB.focus  )
                {
                    SDL_SetRenderDrawColor(ren, dTB.bg.r, dTB.bg.g, dTB.bg.b, dTB.bg.a);
                    SDL_RenderDrawRect(ren, &dTB.bg_rect);      // Draw green glow box
                    SDL_SetRenderDrawColor(ren, dTB.bg.r, dTB.bg.g, dTB.bg.b, 100);
                    SDL_RenderFillRect(ren, &dTB.bg_rect);      // Fill green glow box
                }
                // Render text
                SDL_RenderCopy(ren, dTB.tex, NULL, &dTB.fg_rect);
                SDL_RenderCopy(ren, dCB.tex, NULL, &dCB.fg_rect);
                for( int i=0; i<NUM_CTRLS; i++)
                {
                    SDL_RenderCopy(ren, cS->tex[i], NULL, &(cS->fg_rect[i]));
                }
                SDL_DestroyTexture(dTB.tex);
                SDL_DestroyTexture(dCB.tex);
                for( int i=0; i<NUM_CTRLS; i++)
                {
                    SDL_DestroyTexture(cS->tex[i]);
                }
            }
        }
        { // Isometric

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
