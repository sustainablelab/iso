/* *************TODO***************
 * 1. Isometric mapping!
 * 2. Add a save to export control values to a file
 * 3. I like my controls:
 *    - just add a line to the CTRL_TABLE and I have a new control
 *    - access the value in the control with cS->val[NAME]
 *    - (NAME is the first column in the table)
 *    But I don't like the stand-alone textbox objects.
 * *******************************/
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "main.h"                                               // func prototypes
#include "window_info.h"
#include "print.h"
#include "font.h"
#include "debug_text.h"
#include "text_box.h"
#include "help.h"
#include "controls.h"
#include "line.h" // Replace this with vec.h
#include "vec.h" // Replace this with point.h
#include "point.h"


// Singletons
SDL_Window *win;                                                // The window
SDL_Renderer *ren;                                              // The renderer
TTF_Font *debug_font;                                           // Debug overlay font
Ctrl_SOA *cS;                                                   // Debug controls are SOA
SDL_Texture *Tex_top;                                           // Texture for top-view

#define MODE_TABLE                                                                        \
    /* index */                                                                           \
    X( GAME_MODE,          "Esc to quit"            )/* No controls have focus          */\
    X( DEBUG_WINDOW_MODE,  "Esc to GAME mode"       )/* Normal mode: focus is visible   */\
    X( DEBUG_INSERT_MODE_i,""                       )/* Swallow `i` keypress for insert */\
    X( DEBUG_INSERT_MODE,  "Esc to exit INSERT mode")/* Actual insert mode              */

#define X(index, help) index,
enum mode_index { MODE_TABLE };
#undef X

#define X(index, help) #index " ("help")",
const char *mode_labels[]  = { MODE_TABLE };
#undef X

void shutdown(void)
{
    ctrl_free(cS);
    TTF_CloseFont(debug_font);
    TTF_Quit();
    SDL_DestroyTexture(Tex_top);
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
    win = SDL_CreateWindow(
            argv[0], wI.x, wI.y, wI.w, wI.h, wI.flags);         // Create the window
    ren = SDL_CreateRenderer(win, -1, 0);                       // Create the renderer
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);       // Draw with alpha
    if(  font_setup(&debug_font) < 0  ) return EXIT_FAILURE;    // Init TTF and load font

    Tex_top = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_TARGET,
                                wI.w, wI.h);
    if(SDL_SetTextureBlendMode(Tex_top, SDL_BLENDMODE_BLEND) < 0) // Draw on transparent bgnd
    { // Texture blending is not supported
        puts("Cannot set texture to blendmode blend.");
        shutdown();
        return EXIT_FAILURE;
    }
    /* if(SDL_SetTextureAlphaMod(Tex_top, 255) < 0)                 // Transparency on transparency */
    /* { // Cannot use alpha modulation */ 
    /*     puts("Texture does not support alpha modulation."); */ 
    /*     shutdown(); */ 
    /*     return EXIT_FAILURE; */ 
    /* } */ 

    // Game state
    enum mode_index mode = GAME_MODE;                           // Be modal
    /* bool do_once = true; */
    /* if(do_once) */
    /* { */
    /*     for( int i=0; i<cnt; i++) */
    /*     { */
    /*         printf("%s,", has_hatch[i]?"T":"F"); */
    /*     } */
    /*     puts("");fflush(stdout); */
    /*     do_once = false; */
    /* } */
    bool quit = false;
    bool show_debug = true;                                     // Start debug visible
    bool show_help = false;                                     // Start with help hidden
    Ctrl_SOA controls_in_debug_overlay;                         // Allocate struct mem
    cS = &controls_in_debug_overlay;                            // Point global at struct
    ctrl_load_table(cS, dT_margin);
    { // Overwrite X,Y max values with window size
        cS->max_val[X1] = wI.w; cS->max_val[Y1] = wI.h;
    }

    // Main debug overlay -- print whatever I want here
    TextBox dTB;                                                // Debug overlay Textbox
    char debug_text_buffer[2048];                               // Max 2048 char overlay
    setup_debug_box(&dTB, debug_text_buffer);                   // Init debug overlay
    // Smaller text window on the left to title the controls
    TextBox dCB;                                                // Debug control box
    char debug_controls[32];                                    // Small debug buffer
    setup_debug_box(&dCB, debug_controls);

    // Game loop
    while(  quit == false  )
    {
        // Update state
        SDL_GetWindowSize(win, &wI.w, &wI.h);                   // Adapt to window resize
        dTB.bg_rect.w = wI.w - dCB.bg_rect.w;                   // w : main debug text

        // UI
        SDL_Keymod kmod = SDL_GetModState();                    // kmod : OR'd modifiers
        { // Filtered (rapid fire keys)
            SDL_PumpEvents();
            const Uint8 *k = SDL_GetKeyboardState(NULL);        // Get all keys
            if(  mode == GAME_MODE  )
            {
                if(  k[SDL_SCANCODE_UP]  )   { cS->val[Y1]--; }
                if(  k[SDL_SCANCODE_DOWN]  ) { cS->val[Y1]++; }
                if(  k[SDL_SCANCODE_LEFT]  ) { cS->val[X1]--; }
                if(  k[SDL_SCANCODE_RIGHT]  ){ cS->val[X1]++; }
            }
            else
            {
                if(  !(kmod & KMOD_SHIFT) && !(kmod & KMOD_ALT)  )   // Shift/Alt : ignore arrows
                {
                    if(  k[SDL_SCANCODE_UP]  ) { ctrl_inc(cS, 1); }  // Up Arrow : inc val
                    if(  k[SDL_SCANCODE_DOWN]  ) { ctrl_dec(cS, 1); }// Dn Arrow : dec val
                    if(  k[SDL_SCANCODE_UP] || k[SDL_SCANCODE_DOWN]  )
                    { // Handle mode logic
                        if(  mode == DEBUG_INSERT_MODE  ) { mode = DEBUG_WINDOW_MODE; }
                    }
                }
            }
        }
        { // Polled (delayed repeat fire after initial key press)
            SDL_Event e;
            while(  SDL_PollEvent(&e)  )
            {
                if(  e.type == SDL_TEXTINPUT  )
                {
                    if(  mode == DEBUG_INSERT_MODE  ) { ctrl_buff_in(cS, e.text.text); }
                    else if(  mode == DEBUG_INSERT_MODE_i  ) { mode = DEBUG_INSERT_MODE; }
                }
                if(  e.type == SDL_KEYDOWN  )
                {
                    switch(  e.key.keysym.sym  )
                    {
                        case SDLK_SLASH:                        // ? : Toggle help
                            if(  kmod & KMOD_SHIFT  ) { show_help = !show_help; }
                            break;
                        case SDLK_UP:
                            if(  kmod & KMOD_SHIFT  ) { ctrl_inc(cS, 10); }
                            if(  kmod & KMOD_ALT    ) { ctrl_inc(cS, 1); }
                            break;
                        case SDLK_DOWN:
                            if(  kmod & KMOD_SHIFT  ) { ctrl_dec(cS, 10); }
                            if(  kmod & KMOD_ALT    ) { ctrl_dec(cS, 1); }
                            break;
                        case SDLK_BACKSPACE:                    // Back : del last char
                            if(  mode == DEBUG_INSERT_MODE  ) { ctrl_buff_del(cS); }
                            break;
                        case SDLK_RETURN:                       // Enter : use input as val
                            if(  mode == DEBUG_INSERT_MODE  )
                            { // Hitting Enter doesn't take you out of insert mode
                                ctrl_enter_val(cS);             // Submit val
                                // Shift-Enter takes you to next input field
                                if(  kmod & KMOD_SHIFT  ) { ctrl_focus_next(cS); }
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
                                    ctrl_lose_focus(cS);
                                    break;
                                case DEBUG_INSERT_MODE: case DEBUG_INSERT_MODE_i:
                                    mode = DEBUG_WINDOW_MODE;
                                    break;
                                default: quit = true; break;
                            }
                            break;
                        case SDLK_TAB:
                            show_debug = !show_debug;           // Tab: toggle debug vis
                            break;
                        case SDLK_SEMICOLON:
                            if(  kmod & KMOD_SHIFT  )
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
                                else ctrl_who_has_focus(&TheFocus, cS); // Search the controls
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
                        case SDLK_j:                            // j : Move focus down
                            if(  mode == DEBUG_WINDOW_MODE  )
                            {
                                if(  dCB.focus  )
                                {
                                    dCB.focus = false;
                                    cS->focus[0] = true;
                                }
                                else if(  !(dCB.focus || dTB.focus)  ) { ctrl_focus_next(cS); }
                            }
                            break;
                        case SDLK_k:                            // k : Move focus up
                            if(  mode == DEBUG_WINDOW_MODE  )
                            {
                                if(  cS->focus[0]  )
                                {
                                    cS->focus[0] = false;
                                    dCB.focus = true;
                                }
                                else { ctrl_focus_prev(cS); }
                            }
                            break;
                        case SDLK_i:                            // i : Enter insert mode
                            if(  (mode == DEBUG_WINDOW_MODE) && ctrl_has_focus(cS)  )
                            {
                                mode = DEBUG_INSERT_MODE_i;
                            }
                            break;
                        default: break;
                    }
                }
            }
        }

        // Render
        { // Background (bgnd alpha channel has no effect because bgnd is the bottom layer)
            SDL_SetRenderDrawColor(ren, 20,15,40,0);            // Dark blue-purple
            SDL_RenderClear(ren);
        }
        if(  show_debug  )
        { // Debug overlay
            { // Fill main overlay text buffer with characters
                char *d = dTB.text;                             // d : see macro "print"
                print("Win: "); printint(4,wI.w); print("x"); printint(4,wI.h); print("  ");
                print(mode_labels[mode]);
                /* print("NUM_CTRLS: "); printint(2,NUM_CTRLS); print("\n"); */
                print("\n");
                if(  show_help  ) { print(help_text); }
                else { print(hint_text); }
            }
            { // Fill debug control title text buffer with characters
                char *d = dCB.text;                             // d : see macro "print"
                print("Controls ("); printint(3,NUM_CTRLS);print(")\n");
                print("------------\n");
            }
            { // Fill controls with text: labels and either values or text input buffer
                ctrl_print_val(cS);                                 // Print all the values
                if(  mode == DEBUG_INSERT_MODE  )                   // If in insert mode
                { // Print over value with the input buffer of the control in focus
                    ctrl_print_input_in_focus(cS);                  // Print the input buffer
                }
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
            { // Draw the control text inputs to their textures

                // Draw text to texture
                SDL_Color insert_color = dT_normal_color;       // All text is normal color
                if(  mode == DEBUG_INSERT_MODE  )               // But in insert mode,
                { insert_color = dT_insert_color; }             // use insert color on focus
                ctrl_draw_text(ren, cS, debug_font, dT_normal_color, insert_color);

                // Layout text rects
                int fg_yoffset = dT_margin + dCB.bg_rect.h;     // fg_y for first control
                int bg_w = dCB.bg_rect.w;                       // bg matches title width
                ctrl_make_layout(cS, bg_w, fg_yoffset);         // Layout text rects
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
                    int c = dCB.fg_rect.h + NUM_CTRLS*(cS->fg_rect[0].h) + dT_margin;
                    tallest = (t > c) ? t : c;
                }
                SDL_Rect bg_rect = {.x=0, .y=0, .w=wI.w, .h=(tallest + 2*dT_margin)};
                SDL_RenderFillRect(ren, &bg_rect);              // Draw black bgnd
                SDL_Color focus_color = dT_glow;
                ctrl_render_focus(ren, cS, focus_color);
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
                SDL_DestroyTexture(dTB.tex);
                SDL_DestroyTexture(dCB.tex);
                ctrl_render_text(ren, cS);
            }
        }
        { // Isometric
            /* *************ISO***************
             * Draw a simple L-shaped platform
             *
             * Represent it top-down -- then map it to iso coordinates
             * ----------------------   ------------------------------
             *      0  1  2  3  4  x
             *      |  |  |  |  |
             * 0 ── ┌────────────                       0  0
             *      │                                    /\
             * 1 ── │  ┌─────────                     1 /  \ 1
             *      │  │                               /    \
             * 2 ── │  │                            2 /  /\  \ 2
             *      │  │                             /  /  \  \
             * 3 ── │  │                          3 /  /    \  \ 3
             *      │  │                           /  /      \  \
             * 4 ── │  │                        4 /  /        \  \ 4
             * y    │  │                       y /  /          \  \ x
             * *******************************/
            { // Draw rect controlled by overlay
                SDL_SetRenderDrawColor(ren, cS->val[R],cS->val[G],cS->val[B],cS->val[A]);
                SDL_Rect origin = {cS->val[X1]-cS->val[W]/2, cS->val[Y1]-cS->val[H]/2, cS->val[W], cS->val[H]};
                SDL_RenderDrawRect(ren, &origin);
            }
            // Line color for floor
            /* SDL_SetRenderDrawColor(ren, 150,120,120,200);       // Line color for floor */
            /* SDL_SetRenderDrawColor(ren, 170,51,233,174);         // Live purple : path */
            /* SDL_SetRenderDrawColor(ren,  97,51,233,174);         // Muted purple : vertical lines */
            { // Points
                int s = cS->val[S];
                SDL_Point points[] = {                          // has_hatch
                    (SDL_Point){ 0*s, 0*s},                           // false
                    (SDL_Point){ 0*s, 3*s},                          // true
                    (SDL_Point){ 1*s, 3*s},                         // false
                    (SDL_Point){ 1*s, 4*s},                         // false
                    (SDL_Point){ 0*s, 4*s},                         // false
                    (SDL_Point){ 0*s,10*s},                         // true
                    (SDL_Point){ 1*s,10*s},                         // true
                    (SDL_Point){ 1*s, 5*s},                        // true
                    (SDL_Point){ 2*s, 5*s},                      // true
                    (SDL_Point){ 2*s, 2*s},                         // true
                    (SDL_Point){ 1*s, 2*s},                         // false
                    (SDL_Point){ 1*s, 1*s},                         // true
                    (SDL_Point){10*s, 1*s},                         // true
                    (SDL_Point){10*s, 0*s},                         // true
                    (SDL_Point){ 0*s, 0*s}                            // false
                    };
                int cnt = (int)(sizeof(points)/sizeof(SDL_Point));
                bool has_hatch[] = {                            // Temporary hack
                    false,
                    true,
                    false,
                    false,
                    false,
                    true,
                    true,
                    true,
                    true,
                    true,
                    false,
                    true,
                    true,
                    true,
                    false,
                    };
                if ( 0 )
                { // Needs work
                    //If I do figure out how to do it this way, remember to free(has_hatch)
                    /* free(has_hatch); */

                    /* *************DOC***************
                     * Argh, the algorithm below works, but
                     * it only catches points directly above other points
                     * That is NOT what I need to test for....
                     * *******************************/

                    // Decide who gets a hatch:
                    // If multiple points have same x/y ratio, only biggest point gets hatch line
                    bool *has_hatch = malloc(cnt*sizeof(bool)); // Who has a hatch
                    for( int i=0; i<(cnt-1); i++) { has_hatch[i] = true; }
                    for( int i=0; i<(cnt-1); i++)               // -1 because end point is start point
                    {
                        int a = points[i].x, b = points[i].y;   // This point
                        // Turn 0,0 into 1,1, just for this calculation
                        if(  (a == 0) && (b == 0)  ) { a = 1; b = 1; }
                        for( int j=0; j<(cnt-1); j++)
                        { // Compare this point with all others
                            if(  i!=j  )                        // Don't compare point with itself
                            {
                                int c = points[j].x, d = points[j].y;// Another point
                                // Turn 0,0 into 1,1, just for this calculation
                                if(  (c==0) && (d==0)  ) { c = 1; d = 1; }
                                if(  (a*d) == (b*c)  )
                                { // These points are on the same vertical!
                                    if(a<c)
                                    {
                                        has_hatch[i] = false;   // Do not hatch this point
                                        break;                  // Done checking this point
                                    }
                                }
                            }
                        }
                    }
                    has_hatch[(cnt-1)] = has_hatch[0];          // because end point is start point
                }
                { // Render top-view
                    // Define region to draw and where to draw it
                    // src_rect : the portion of Tex_top I want to use
                    // dst_rect : the scale and location to use Tex_top on the screen
                    SDL_Rect src_rect = {0};                    // Find box bounding points
                    SDL_EnclosePoints(points, cnt, NULL, &src_rect);
                    SDL_Rect dst_rect = src_rect;               // Scale 1 : 1
                    dst_rect.x = 50; dst_rect.y = cS->val[Y1];  // Put top-view left of iso-view
                    // Render
                    SDL_SetRenderTarget(ren, Tex_top);          // Render to this texture
                    SDL_SetRenderDrawColor(ren, 0,0,0,0);       // Start with blank texture
                    SDL_RenderClear(ren);
                    SDL_SetRenderDrawColor(ren, 150,60,140,115);// Line color for ghosted top-view
                    SDL_RenderDrawLines(ren, points, cnt);      // Connect the dots
                    SDL_SetRenderTarget(ren, NULL);             // Set renderer back to screen
                    SDL_RenderCopy(ren, Tex_top, &src_rect, &dst_rect);
                }

                SDL_Point offset = {cS->val[X1], cS->val[Y1]};  // Translate origin 0,0
                for( int i=0; i<cnt; i++)
                {
                    point_map_top_to_iso(&points[i]);
                    point_move(&points[i], offset);
                }
                SDL_SetRenderDrawColor(ren, 170,51,233,174);         // Live purple : path
                /* SDL_SetRenderDrawColor(ren,  97,51,233,174);         // Muted purple : vertical lines */
                /* SDL_SetRenderDrawColor(ren, 150,60,140,115);// Line color for ghosted top-view */
                SDL_RenderDrawLines(ren, points, cnt);      // Connect the dots
                { // Hatch lines (drop vertical from each point)
                    int len = 10; // cS->val[S]*3;
                    SDL_SetRenderDrawColor(ren,  97,51,233,130);         // Muted purple : vertical lines

                    for( int i=0; i<cnt; i++)
                    {
                        if(  has_hatch[i]  )
                        {
                            int x = points[i].x; int y = points[i].y;
                            Line l = {x, y, x, y+len};
                            line_draw(ren, l);
                        }
                    }
                }
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
