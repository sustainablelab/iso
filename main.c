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
#include "aff.h"

bool do_once = true;
int cnt_down = 20;

// Singletons
SDL_Window *win;                                                // The window
SDL_Renderer *ren;                                              // The renderer
TTF_Font *debug_font;                                           // Debug overlay font
Ctrl_SOA *cS;                                                   // Debug controls are SOA
SDL_Texture *Tex_top;                                           // Texture for top-view

SDL_Rect SDL_RECT;                                              // Print some rect
SDL_FRect SDL_FRECT;                                            // Print some float rect
SDL_FPoint SDL_FPOINT;                                          // Print some float point
int INT, INT2;                                                  // Print some ints
AffSeg AFFSEG;                                                  // Print some line seg
AffLine AFFLINE_I, AFFLINE_J;                                   // Print some lines

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
            if(  !(kmod & KMOD_SHIFT) && !(kmod & KMOD_ALT)  )   // Shift/Alt : ignore arrows
            {
                if(  mode == GAME_MODE  )
                {
                    if(  k[SDL_SCANCODE_UP]  )   { cS->val[Y1]--; }
                    if(  k[SDL_SCANCODE_DOWN]  ) { cS->val[Y1]++; }
                    if(  k[SDL_SCANCODE_LEFT]  ) { cS->val[X1]--; }
                    if(  k[SDL_SCANCODE_RIGHT]  ){ cS->val[X1]++; }
                }
                else
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
                        case SDLK_LEFT:
                            if(  kmod & KMOD_ALT  )
                            {
                                cS->val[I]--;
                                if (cS->val[I]<0) { cS->val[I] = 0; }
                            }
                            if(  kmod & KMOD_SHIFT  )
                            {
                                cS->val[J]--;
                                if (cS->val[J]<0) { cS->val[J] = 0; }
                            }
                            break;
                        case SDLK_RIGHT:
                            if(  kmod & KMOD_ALT  )
                            {
                                cS->val[I]++;
                                if (cS->val[I]>cS->max_val[I]) { cS->val[I] = cS->max_val[I]; }
                            }
                            if(  kmod & KMOD_SHIFT  )
                            {
                                cS->val[J]++;
                                if (cS->val[J]>cS->max_val[J]) { cS->val[J] = cS->max_val[J]; }
                            }
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
                if(0)
                { // SDL_FRECT
                    print("EncloseFPoints fr: {");
                    printfloat(SDL_FRECT.x);print(",");printfloat(SDL_FRECT.y);print(",");
                    printfloat(SDL_FRECT.w);print(",");printfloat(SDL_FRECT.h);print("}");
                    print("\n");
                }
                { // SDL_RECT
                    print("Top-view dst_rect: {");
                    printint(6, SDL_RECT.x);print(",");printint(6, SDL_RECT.y);print(",");
                    printint(6, SDL_RECT.w);print(",");printint(6, SDL_RECT.h);print("}");
                    print("\n");
                }
                { // SDL_FPOINT
                    print("Offset fpoint: {");
                    printfloat(SDL_FPOINT.x);print(",");printfloat(SDL_FPOINT.y);print("}");
                    print("\n");
                }
                if(0)
                { // INT
                    print("Points in map: ");printint(4,INT);print("\n");
                    print("Segs in map: ");  printint(4,INT2);print("\n");
                }
                { // AFFSEG
                    float ax = AFFSEG.A.x; float ay = AFFSEG.A.y;
                    float bx = AFFSEG.B.x; float by = AFFSEG.B.y;
                    print("Seg path_border[");printint(4,cS->val[I]);print("]: ");
                    print("A(");    printfloat(ax);print(",");printfloat(ay);
                    print("), B("); printfloat(bx);print(",");printfloat(by);
                    print(")\n");
                }
                { // AFFLINE
                    float Ia = AFFLINE_I.a; float Ib = AFFLINE_I.b; float Ic = AFFLINE_I.c;
                    float Ja = AFFLINE_J.a; float Jb = AFFLINE_J.b; float Jc = AFFLINE_J.c;
                    print("Depth line [I]: ");
                    printfloat(Ia);print(",");printfloat(Ib);print(",");printfloat(Ic);
                    print(")\n");
                    print("Path line [J]: ");
                    printfloat(Ja);print(",");printfloat(Jb);print(",");printfloat(Jc);
                    print(")\n");
                }
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
            // Line color for floor
            /* SDL_SetRenderDrawColor(ren, 150,120,120,200);       // Line color for floor */
            /* SDL_SetRenderDrawColor(ren, 170,51,233,174);         // Live purple : path */
            /* SDL_SetRenderDrawColor(ren,  97,51,233,174);         // Muted purple : vertical lines */
            { // Points
                SDL_FPoint offset = {cS->val[X1], cS->val[Y1]};  // Translate origin 0,0
                SDL_FPOINT.x = offset.x; SDL_FPOINT.y = offset.y;

                float s = (float)(cS->val[S]);
                // Simple shape for debug
                /* AffPoint points[] = { */
                /*     (AffPoint){ 0*s, 0*s}, */
                /*     (AffPoint){ 0*s, 5*s}, */
                /*     (AffPoint){ 4*s, 5*s}, */
                /*     (AffPoint){ 4*s, 0*s}, */
                /*     (AffPoint){ 0*s, 0*s} */
                /*     }; */
                // TODO: switch back to fancier shape
                AffPoint points[] = {
                    (AffPoint){ 0*s, 0*s},
                    (AffPoint){ 0*s, 3*s},
                    (AffPoint){ 1*s, 3*s},
                    (AffPoint){ 1*s, 4*s},
                    (AffPoint){ 0*s, 4*s},
                    (AffPoint){ 0*s,10*s},
                    (AffPoint){ 1*s,10*s},
                    (AffPoint){ 1*s, 5*s},
                    (AffPoint){ 2*s, 5*s},
                    (AffPoint){ 2*s, 2*s},
                    (AffPoint){ 1*s, 2*s},
                    (AffPoint){ 1*s, 1*s},
                    (AffPoint){10*s, 1*s},
                    (AffPoint){10*s, 0*s},
                    (AffPoint){ 0*s, 0*s}
                    };
                int cnt = (int)(sizeof(points)/sizeof(AffPoint));

                // Define "path border"
                int cnt_seg = cnt-1;                            // Number of line segments
                cS->max_val[I] = cnt_seg-1;
                cS->max_val[J] = cnt_seg-1;
                AffSeg *path_border = malloc(sizeof(AffSeg)*cnt_seg);
                { // Define path border
                    // Go through the points. For each pair of points (A,B):
                    // make a directed line segment sAB (from A to B)
                    for(int i=0; i<cnt_seg; i++)
                    {
                        path_border[i] = (AffSeg){points[i],points[i+1]};
                    }
                AFFSEG = path_border[cS->val[I]];
                }

                // Vertical depth artwork
                AffSeg *depth_art = malloc(sizeof(AffSeg)*cnt_seg);
                float depth = (float)(cS->val[D]);
                { // Define vertical depth lines
                    // Go through the points. For each point A:
                    // make a directed line segment sAZ (from A to Z)
                    // Z is a point directly below A by the amount "depth"
                    for(int i=0; i<cnt_seg; i++)
                    {
                        AffPoint A = points[i];
                        AffPoint Z = {(A.x+depth), (A.y+depth)};    // 45° maps to vertical
                        depth_art[i] = (AffSeg){A,Z};
                    }
                }
                // No need to check for clipping if seg.B is not inside the path polygon
                // How do I check that?

                if(1)
                { // Clip vertical depth lines where visually obscured by path border
                    /* *************DOC***************
                     * Path border is an array of AffSeg.
                     * Vertical depth artwork is in an array of AffSeg
                     * For each vertical depth line:
                     * Does the line intersect any path border segment?
                     * - Turn both line segments into lines
                     * - Find where the lines intersect
                     * - Check if this point lies on the vertical depth line
                     * - if so, check if this point also lies on the border segment
                     * - if both are true, then change the second point of the vertical line
                     *   segment to be this intersection point
                     * *******************************/
                    // Make array of lines once to reuse it in the loop
                    AffLine *path_lines = malloc(sizeof(AffLine)*cnt_seg);
                    for(int i=0; i<cnt_seg; i++)
                    {
                        path_lines[i] = aff_join_of_points(path_border[i].A, path_border[i].B);
                    }
                    AffLine *depth_lines = malloc(sizeof(AffLine)*cnt_seg);
                    for(int i=0; i<cnt_seg; i++)
                    {
                        depth_lines[i] = aff_join_of_points(depth_art[i].A, depth_art[i].B);
                    }
                    // DEBUG
                    AFFLINE_I = depth_lines[cS->val[I]];
                    AFFLINE_J = path_lines[cS->val[J]];
                    // END DEBUG
                    // Iterate over the depth art segments
                    for(int i=0; i<cnt_seg; i++)
                    {
                        int M_cnt = 0;                          // DEBUG: cnt number of intersections
                        for(int j=0; j<cnt_seg; j++)
                        {
                            // Find intersection with each AffLine in path_lines
                            AffPoint M = aff_meet_of_lines(depth_lines[i], path_lines[j]); // point of intersection
                            if(1)
                            { // DEBUG -- render intersection point
                                // Draw M in top-view
                                // src_rect : the portion of Tex_top I want to use
                                // dst_rect : the scale and location to use Tex_top on the screen
                                SDL_FRect fr = {0};                         // Find box bounding points
                                SDL_EncloseFPoints(points, cnt, NULL, &fr);
                                SDL_Rect src_rect = {(int)fr.x, (int)fr.y, (int)fr.w+depth, (int)fr.h+depth};
                                SDL_Rect dst_rect = src_rect;               // Scale 1 : 1
                                dst_rect.x = 50; dst_rect.y = cS->val[Y1];  // Put top-view left of iso-view
                                // Render
                                SDL_SetRenderTarget(ren, Tex_top);          // Render to this texture
                                SDL_SetRenderDrawColor(ren, 0,0,0,0);       // Start with blank texture
                                SDL_RenderClear(ren);
                                { // Render M in top
                                    SDL_SetRenderDrawColor(ren, cS->val[R],0,0,cS->val[A]);
                                    int x = (int)(M.x); int y = (int)(M.y);
                                    SDL_Rect origin = {x - 2, y - 2, 4, 4};
                                    SDL_RenderDrawRect(ren, &origin);
                                }
                                SDL_SetRenderTarget(ren, NULL);             // Set renderer back to screen
                                SDL_RenderCopy(ren, Tex_top, &src_rect, &dst_rect);
                                // Draw M in iso view
                                AffPoint isoM = M;              // Copy M
                                point_fmap_top_to_iso(&isoM);
                                point_fmove(&isoM,offset);
                                { // Render isoM in iso
                                    if(  cS->val[I] == i  )
                                    {SDL_SetRenderDrawColor(ren, cS->val[R],50,200,cS->val[A]);}
                                    else if(  cS->val[J] == j  )
                                    {SDL_SetRenderDrawColor(ren, 0,200,200,cS->val[A]);}
                                    else
                                    {SDL_SetRenderDrawColor(ren, 255,255,255,5);}
                                    int x = (int)(isoM.x); int y = (int)(isoM.y);
                                    if(  cS->val[I] == i  )
                                    {
                                        SDL_Rect origin = {x - 4, y - 4, 8, 8};
                                        SDL_RenderDrawRect(ren, &origin);
                                    }
                                    else if(  cS->val[J] == j  )
                                    {
                                        SDL_Rect origin = {x - 3, y - 3, 6, 6};
                                        SDL_RenderDrawRect(ren, &origin);
                                    }
                                    else
                                    {
                                        SDL_Rect origin = {x - 2, y - 2, 4, 4};
                                        SDL_RenderDrawRect(ren, &origin);
                                    }
                                    
                                }
                                // END DEBUG
                            }
                            // Does this point lie on both line segments?
                            AffSeg ds = depth_art[i];           // depth seg
                            AffSeg ps = path_border[j];         // path seg
                            if(do_once)
                            {
                                printf("M : on path seg %d: %s\t",j,aff_point_on_seg(M,ps)?"T":"F");
                                printf("on depth seg %d: %s\t",i,aff_point_on_seg(M,ds)?"T":"F");
                                printf("on both? %s\n", (  aff_point_on_seg(M, ps) && aff_point_on_seg(M, ds)  )?"T":"F");
                                /* cnt_down--; if(cnt_down == 0) do_once = false; */
                            }
                            if(  aff_point_on_seg(M, ps) && aff_point_on_seg(M, ds)  )
                            { // The border path segment and depth line segment intersect at M
                                if(0) // TODO: figure out why this is wrong
                                {
                                    // This is definitely the wrong logic.
                                    // The number of intersections doesn't make any sense.
                                    depth_art[i].B = M;         // Clip depth art at intersection
                                }
                                M_cnt++;                        // DEBUG
                            }
                        }
                        if(do_once)
                        {
                            printf("M_cnt : depth seg %d: %d\n",i,M_cnt);fflush(stdout);
                            cnt_down--; if(cnt_down == 0) do_once = false;
                        }
                    }
                    free(path_lines);                           // Done with these lines
                    free(depth_lines);                          // Done with these lines
                }

                { // Render top-view
                    // Define region to draw and where to draw it
                    // src_rect : the portion of Tex_top I want to use
                    // dst_rect : the scale and location to use Tex_top on the screen
                    SDL_FRect fr = {0};                         // Find box bounding points
                    SDL_EncloseFPoints(points, cnt, NULL, &fr);
                    SDL_Rect src_rect = {(int)fr.x, (int)fr.y, (int)fr.w+depth, (int)fr.h+depth};
                    SDL_Rect dst_rect = src_rect;               // Scale 1 : 1
                    dst_rect.x = 50; dst_rect.y = cS->val[Y1];  // Put top-view left of iso-view
                    SDL_RECT.x = dst_rect.x; SDL_RECT.y = dst_rect.y;
                    SDL_RECT.w = dst_rect.w; SDL_RECT.h = dst_rect.h;
                    // Render
                    SDL_SetRenderTarget(ren, Tex_top);          // Render to this texture
                    SDL_SetRenderDrawColor(ren, 0,0,0,0);       // Start with blank texture
                    SDL_RenderClear(ren);
                    SDL_SetRenderDrawColor(ren, 150,60,140,150);// Line color for ghosted top-view
                    SDL_RenderDrawLinesF(ren, points, cnt);      // Connect the dots
                    { // Hatch lines (drop vertical from each point)
                        /* SDL_SetRenderDrawColor(ren,  97,51,233,150); // Muted purple : vertical lines */
                        SDL_SetRenderDrawColor(ren,  97,51,233,255); // Muted purple : vertical lines
                        for(int i=0; i<cnt_seg; i++)
                        {
                            AffSeg seg = depth_art[i];
                            SDL_RenderDrawLineF(ren, seg.A.x, seg.A.y, seg.B.x, seg.B.y);
                        }
                    }
                    { // Draw rect controlled by overlay -- cool looks like walking thing
                        SDL_SetRenderDrawColor(ren, cS->val[R],cS->val[G],cS->val[B],cS->val[A]);
                        /* SDL_Rect origin = {cS->val[X1]-cS->val[W]/2, cS->val[Y1]-cS->val[H]/2, cS->val[W], cS->val[H]}; */
                        { // seg.A
                            int x = (int)(AFFSEG.A.x); int y = (int)(AFFSEG.A.y);
                            SDL_Rect origin = {x - cS->val[W]/2, y - cS->val[H]/2, cS->val[W], cS->val[H]};
                            SDL_RenderDrawRect(ren, &origin);
                        }
                        { // seg.B
                            int x = (int)(AFFSEG.B.x); int y = (int)(AFFSEG.B.y);
                            SDL_Rect origin = {x - cS->val[W]/2, y - cS->val[H]/2, cS->val[W], cS->val[H]};
                            SDL_RenderDrawRect(ren, &origin);
                        }
                    }
                    SDL_SetRenderTarget(ren, NULL);             // Set renderer back to screen
                    SDL_RenderCopy(ren, Tex_top, &src_rect, &dst_rect);
                }

                for( int i=0; i<cnt; i++)
                { // map from top to iso
                    point_fmap_top_to_iso(&points[i]);
                    point_fmove(&points[i], offset);
                }
                SDL_SetRenderDrawColor(ren, 170,51,233,174);         // Live purple : path
                /* SDL_SetRenderDrawColor(ren,  97,51,233,174);         // Muted purple : vertical lines */
                /* SDL_SetRenderDrawColor(ren, 150,60,140,115);// Line color for ghosted top-view */
                SDL_RenderDrawLinesF(ren, points, cnt);      // Connect the dots
                { // Hatch lines (drop vertical from each point)
                    for( int i=0; i<cnt_seg; i++)
                    { // map from top to iso
                        point_fmap_top_to_iso(&depth_art[i].A);
                        point_fmap_top_to_iso(&depth_art[i].B);
                        point_fmove(&depth_art[i].A, offset);
                        point_fmove(&depth_art[i].B, offset);
                    }
                    SDL_SetRenderDrawColor(ren,  97,51,233,130);         // Muted purple : vertical lines
                    for(int i=0; i<cnt_seg; i++)
                    {
                        AffSeg seg = depth_art[i];
                        SDL_RenderDrawLineF(ren, seg.A.x, seg.A.y, seg.B.x, seg.B.y);
                    }
                }
                if(1)
                { // Draw rect controlled by overlay -- cool looks like walking thing
                    SDL_SetRenderDrawColor(ren, cS->val[R],cS->val[G],cS->val[B],cS->val[A]);
                    /* SDL_Rect origin = {cS->val[X1]-cS->val[W]/2, cS->val[Y1]-cS->val[H]/2, cS->val[W], cS->val[H]}; */
                    SDL_Point offset = {cS->val[X1], cS->val[Y1]};  // Translate origin 0,0
                    { // seg.A
                        int x = (int)(AFFSEG.A.x); int y = (int)(AFFSEG.A.y);
                        // map from top to iso
                        SDL_Point o = {x,y};
                        point_map_top_to_iso(&o);
                        point_move(&o, offset);
                        SDL_Rect origin = {o.x - cS->val[W]/2, o.y - cS->val[H]/2, cS->val[W], cS->val[H]};
                        SDL_RenderDrawRect(ren, &origin);
                    }
                    { // seg.B
                        int x = (int)(AFFSEG.B.x); int y = (int)(AFFSEG.B.y);
                        // map from top to iso
                        SDL_Point o = {x,y};
                        point_map_top_to_iso(&o);
                        point_move(&o, offset);
                        SDL_Rect origin = {o.x - cS->val[W]/2, o.y - cS->val[H]/2, cS->val[W], cS->val[H]};
                        SDL_RenderDrawRect(ren, &origin);
                    }
                }
                free(path_border);
                free(depth_art);
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
