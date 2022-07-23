#ifndef __CONTROLS_H__
#define __CONTROLS_H__

/* *************Usage (how I'm using it for now)***************
 * main.c declares a global Ctrl_SOA pointer:
 *
 * Ctrl_SOA *cS;                                                // Debug controls are SOA
 *
 * main.c does static struct mem allocation and points global at that mem, like this:
 *
 * main(){ ...
 *     Ctrl_SOA controls_in_debug_overlay;                      // Allocate struct mem
 *      cS = &controls_in_debug_overlay;                        // Point global at struct
 * *******************************/
#include <stdbool.h>
#include <SDL.h>

// Add a control by adding an entry to this table. That's it.
#define CTRL_TABLE                              \
    /*    index,label,  val, max_val, min_val */\
        X(  R,  "R: ",  200,     255,       0)  \
        X(  G,  "G: ",  130,     255,       0)  \
        X(  B,  "B: ",    0,     255,       0)  \
        X(  A,  "A: ",  255,     255,       0)  \
        X( X1,  "X1: ", 100,     400,       0)  \
        X( Y1,  "Y1: ", 350,     400,       0)  \
        X( X2,  "X2: ", 200,     400,       0)  \
        X( Y2,  "Y2: ", 250,     400,       0)

// Preprocessor output:
// enum controls_index { R, G, B, A, X, Y};
#define X(index, label, val, max_val, min_val) index,
enum controls_index { CTRL_TABLE };
#undef X

// Preprocessor output:
// const char *controls_label[] = { "R: ", "G: ", "B: ", "A: ", "X: ", "Y: "};
#define X(index, label, val, max_val, min_val) label,
const char *controls_label[] = { CTRL_TABLE };
#undef X

// Preprocessor output: int controls_val[] = { 128, 128, 0, 0, 0, 0 };
#define X(index, label, val, max_val, min_val) val,
int controls_val[] = { CTRL_TABLE };
#undef X
#define NUM_CTRLS ( (int)(sizeof(controls_val)/sizeof(int)) )

#define X(index, label, val, max_val, min_val) max_val,
int controls_max_val[] = { CTRL_TABLE };
#undef X

#define X(index, label, val, max_val, min_val) min_val,
int controls_min_val[] = { CTRL_TABLE };
#undef X

// Declare static mem here; zero-init in ctrl_load_table():
bool controls_focus[NUM_CTRLS];
SDL_Rect controls_fg_rect[NUM_CTRLS];
SDL_Rect controls_bg_rect[NUM_CTRLS];

typedef struct
{
    /* SDL_Color bg;                                   // Control bgnd color */
    int *val;                                          // Control value cS->val[i]
    int *max_val;                                      // Control max value cS->max_val[i]
    int *min_val;                                      // Control min value cS->min_val[i]
    // Get the control value from user text input
    char *text[NUM_CTRLS];                             // Buffer for all text
    char *text_end[NUM_CTRLS];                         // Store where text buff ends
    const char **label;                                // Text label (ex "R: ")
    char *buff_in[NUM_CTRLS];                          // Control text input buff
    char *buff_c[NUM_CTRLS];                           // Walk the input buff
    char *buff_end[NUM_CTRLS];                         // Store where buff ends
    SDL_Texture *tex[NUM_CTRLS];                       // Textures
    SDL_Rect *fg_rect;                                 // Text inner bounding box
    SDL_Rect *bg_rect;                                 // Text outer box (margins)
    bool *focus;                                       // Has user's focus cS->focus[i]
} Ctrl_SOA;

void ctrl_load_table(Ctrl_SOA *cS, int left_margin)
{
    { // Static mem stuff
        { // These values are set in the CTRL_TABLE
            cS->label = controls_label;                         // Initialized by X Macro
            cS->val = controls_val;                             // Initialized by X Macro
            cS->max_val = controls_max_val;                     // Initialized by X Macro
            cS->min_val = controls_min_val;                     // Initialized by X Macro
        }
        { // Don't use CTRL_TABLE for these values: unnecessary complexity
            cS->focus = controls_focus;
            for(int i=0; i<NUM_CTRLS; i++) cS->focus[i] = false;// Initialize to false
            cS->fg_rect = controls_fg_rect;
            for(int i=0; i<NUM_CTRLS; i++)
            { // Zero-init y,w,h (they are later adjusted during rendering)
                cS->fg_rect[i].x = left_margin;                 // hardcode x to left_margin
                cS->fg_rect[i].y = 0;
                cS->fg_rect[i].w = 0;
                cS->fg_rect[i].h = 0;
            }
            cS->bg_rect = controls_bg_rect;
            for(int i=0; i<NUM_CTRLS; i++)
            { // Zero-init y,w,h (they are later adjusted during rendering)
                cS->bg_rect[i] = (SDL_Rect){0};                 // hardcode x to 0
            }
        }
    }
    { // Heap mem stuff
        for (int i=0; i<NUM_CTRLS; i++)
        { // Create string buffers for control text inputs

            // text : print all here - label AND user input text
            // Example: "R: 128"
            // - label:             "R: "
            // - user input text:   "128"
            int text_len = 32;                                  // Max 31 chars plus nul
            cS->text[i] = malloc(sizeof(char)*text_len);        // Entire ctrl: 32 chars
            int text_rel_end = (text_len-1)*sizeof(char);       // Relative text buff end
            cS->text_end[i] = cS->text[i] + text_rel_end;       // Point at text buff end

            // buff_in : buffer for user input text
            int len = 8;
            cS->buff_in[i] = malloc(sizeof(char)*len);          // Input: at most 8 char

            // buff_c : walk user input text (char by char)
            cS->buff_c[i] = cS->buff_in[i];                     // Point at buff start
            *(cS->buff_c[i]) = '\0';                            // nul-terminate start

            // buff_end : address of last byte buff_in can hold
            int rel_end = (len-1)*sizeof(char);                 // Relative buff end
            cS->buff_end[i] = cS->buff_in[i] + rel_end;         // Point at buff end
        }
    }
}

void ctrl_free(Ctrl_SOA *cS)
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
}

bool ctrl_has_focus(Ctrl_SOA *cS)
{ // Return true if any control has focus, otherwise false
    bool has_focus = false;
    for( int i=0; i<NUM_CTRLS; i++)
    {
        if(  cS->focus[i]  )
        {
            has_focus = true;
            break;
        }
    }
    return has_focus;
}

void ctrl_who_has_focus(bool **TheFocus, Ctrl_SOA *cS)
{ // Point TheFocus at the control that has focus
    for(int i=0; i<NUM_CTRLS; i++)
    {
        if(  cS->focus[i]  )
        {
            *TheFocus = &(cS->focus[i]);
            break;
        }
    }
}

int _clamp(int val, int max, int min)
{ // Return clamped value.
    /* *************Check max AND min***************
     * When incrementing, clamp at max, but also clamp at min!
     * When decrementing, clamp at min, but also clamp at max!
     *
     * Why:
     * - say user inputs a value way above max
     * - then presses the down arrow to decrement
     * - expected behavior: number immediately jumps back in range
     *
     * If decrement only checks for clamp at min,
     * I get undesired behavior: the arrow key decrements this huge value.
     * *******************************/
    if (  val > max  ) { return max; }
    if (  val < min  ) { return min; }
    return val;
}

void ctrl_inc(Ctrl_SOA *cS)
{
    for( int i=0; i<NUM_CTRLS; i++ )
    {
        if(  cS->focus[i]  )
        {
            cS->val[i]++;
            cS->val[i] = _clamp(cS->val[i], cS->max_val[i], cS->min_val[i]);
            break;
        }
    }
}

void ctrl_dec(Ctrl_SOA *cS)
{
    for( int i=0; i<NUM_CTRLS; i++ )
    {
        if(  cS->focus[i]  )
        {
            cS->val[i]--;
            cS->val[i] = _clamp(cS->val[i], cS->max_val[i], cS->min_val[i]);
            break;
        }
    }
}

void ctrl_buff_in(Ctrl_SOA *cS, const char *c)
{ // Copy text input to buff_in
    /* *************DOC***************
     * c = e.text.text, e : SDL_Event
     * I think e.text.text is always one char.
     * But to play it safe, I treat it like multiple chars.
     * ****************************/
    for( int i=0; i<NUM_CTRLS; i++ )
    {
        if(  cS->focus[i]  )
        {
            while(  (*c!='\0') && (cS->buff_c[i] < cS->buff_end[i])  )
            { *(cS->buff_c[i])++ = *c++; }
            // TODO: is this check for buff_c > buff_end redundant?
            if(  cS->buff_c[i] > cS->buff_end[i]  ) { cS->buff_c[i] = cS->buff_end[i]; }
            *cS->buff_c[i] = '\0';                              // nul-terminate string
            break;
        }
    }
}

void ctrl_buff_del(Ctrl_SOA *cS)
{ // Delete last char from buff_in
    for( int i=0; i<NUM_CTRLS; i++ )
    {
        if(  cS->focus[i]  )
        {
            cS->buff_c[i]--;
            if(  cS->buff_c[i] < cS->buff_in[i]  ) { cS->buff_c[i] = cS->buff_in[i]; }
            *(cS->buff_c[i]) = '\0';                            // nul-terminate string
            break;
        }
    }
}

void ctrl_enter_val(Ctrl_SOA *cS)
{ // Submit buff_in as the new value
    for( int i=0; i<NUM_CTRLS; i++ )
    {
        if(  cS->focus[i]  )
        {
            cS->val[i] = atoi(cS->buff_in[i]);
            break;
        }
    }
}

void ctrl_focus_next(Ctrl_SOA *cS)
{ // Shift focus to next control
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

void ctrl_focus_prev(Ctrl_SOA *cS)
{ // Shift focus to previous control
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

void ctrl_print_val(Ctrl_SOA *cS)
{ // Print all control values
    /* *************DOC***************
     * Print like the print macros in print.h (copies src str to dst str)
     * but here I safeguard against writing past the end of the dst str.
     * *******************************/
    for(int i=0; i<NUM_CTRLS; i++)
    {
        char *d = cS->text[i];                                  // Walk the print buffer
        { // Print label
            const char *c = cS->label[i];                       // Print this string
            while(  (*c!='\0') && (d < cS->text_end[i])  ){ *d++=*c++;} *d='\0';
        }
        { // Print value
            // Convert val to a string ... TODO: get rid of sprintf
            char str[16+1]; sprintf(str, "%d", cS->val[i]);     // Max 16 digits
            const char *c = str;                                // Print this string
            while(  (*c!='\0') && (d < cS->text_end[i])  ){ *d++=*c++;} *d='\0';
        }
    }
}

void ctrl_print_input_in_focus(Ctrl_SOA *cS)
{ // Print the control input buffer that is in focus
    /* *************DOC***************
     * Print like the print macros in print.h (copies src str to dst str)
     * but here I safeguard against writing past the end of the dst str.
     *
     * Call ctrl_print_val() first, otherwise it will overwrite this.
     * This only prints the one control that is in focus.
     * *******************************/
    for(int i=0; i<NUM_CTRLS; i++)
    {
        if(  cS->focus[i]  )
        {
            char *d = cS->text[i];                              // Walk the print buffer
            { // Print label
                const char *c = cS->label[i];                   // Print this string
                while(  (*c!='\0') && (d < cS->text_end[i])  ){ *d++=*c++;} *d='\0';
            }
            { // Print input buffer
                const char *c = cS->buff_in[i];                 // Print this string
                while(  (*c!='\0') && (d < cS->text_end[i])  ){ *d++=*c++;} *d='\0';
            }
            break;
        }
    }
}

void ctrl_draw_text(SDL_Renderer *ren,
                    Ctrl_SOA *cS,
                    TTF_Font *debug_font,
                    SDL_Color normal_color,
                    SDL_Color insert_color)
{ // Draw each control text on its texture
    for( int i=0; i<NUM_CTRLS; i++ )
    {
        // Caller tests for insert mode, I don't worry about that here
        SDL_Color text_color = normal_color;                    // Use normal color
        if(  cS->focus[i]  ) { text_color = insert_color; }     // Use insert color
        SDL_Surface *surf = TTF_RenderText_Blended_Wrapped(
                                debug_font,
                                cS->text[i],                    // Text buffer to render
                                text_color,                     // Text color
                                0);                             // Wrap on new lines
        // Creating the texture determines the fg_rect w and h
        cS->tex[i] = SDL_CreateTextureFromSurface(ren, surf);
        SDL_FreeSurface(surf);
        SDL_QueryTexture(cS->tex[i], NULL, NULL,
                            &(cS->fg_rect[i].w),        // Get text width
                            &(cS->fg_rect[i].h));       // Get text height
    }
}

void ctrl_make_layout(Ctrl_SOA *cS, int bg_w, int fg_yoffset)
{ // Write fgnd rect y, and bgnd rect y,w,h
    /* *************DOC***************
     * Pass these values:
     * bg_w : the bgnd width of the title above the controls
     * fg_yoffset : the height of the title bgnd plus the debug text margin
     * *******************************/
    for( int i=0; i<NUM_CTRLS; i++ )
    {
        // fg_rect w and h are already set by the texture
        // Just need to adjust the fg_rect y value
        cS->fg_rect[i].y = fg_yoffset + i*(cS->fg_rect[i].h);   // y based on index
        // Copy fg_rect y and height to bg_rect
        // Match bg_rect width to title above controls
        cS->bg_rect[i].y = cS->fg_rect[i].y;                    // bg y matches text y
        cS->bg_rect[i].h = cS->fg_rect[i].h;                    // bg height fits text
        cS->bg_rect[i].w = bg_w;                                // bg width matches title
    }
}

#endif // __CONTROLS_H__
