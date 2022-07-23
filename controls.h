#ifndef __CONTROLS_H__
#define __CONTROLS_H__

#include <stdbool.h>
#include <SDL.h>

// Add a control by adding an entry to this table. That's it.
#define CTRL_TABLE              \
    /*    index,label, val  */  \
        X(  R,  "R: ", 128)     \
        X(  G,  "G: ", 128)     \
        X(  B,  "B: ",   0)     \
        X(  A,  "A: ",   0)     \
        X(  X,  "X: ",   0)     \
        X(  Y,  "Y: ",   0)

// Preprocessor output:
// enum controls_index { R, G, B, A, X, Y};
#define X(index, label, val) index,
enum controls_index { CTRL_TABLE };
#undef X

// Preprocessor output:
// const char *controls_label[] = { "R: ", "G: ", "B: ", "A: ", "X: ", "Y: "};
#define X(index, label, val) label,
const char *controls_label[] = { CTRL_TABLE };
#undef X

// Preprocessor output: int controls_val[] = { 128, 128, 0, 0, 0, 0 };
#define X(index, label, val) val,
int controls_val[] = { CTRL_TABLE };
#undef X
#define NUM_CTRLS (int)(sizeof(controls_val)/sizeof(int))

// Declare static mem here; zero-init in ctrl_load_table():
bool controls_focus[NUM_CTRLS];
SDL_Rect controls_fg_rect[NUM_CTRLS];
SDL_Rect controls_bg_rect[NUM_CTRLS];

typedef struct
{
    /* SDL_Color bg;                                              // Control bgnd color */
    int *val;                                                   // Control value cS->val[i]
    // Get the control value from user text input
    char *text[NUM_CTRLS];                                      // Buffer for all text
    const char **label;                                         // Text label (ex "R: ")
    char *buff_in[NUM_CTRLS];                                   // Control text input buff
    char *buff_c[NUM_CTRLS];                                    // Walk the input buff
    char *buff_end[NUM_CTRLS];                                  // Store where buff ends
    SDL_Texture *tex[NUM_CTRLS];                                // Textures
    SDL_Rect *fg_rect;                                          // Text inner bounding box
    SDL_Rect *bg_rect;                                          // Text outer box (margins)
    bool *focus;                                                // Has user's focus cS->focus[i]
} Ctrl_SOA;

void ctrl_load_table(Ctrl_SOA *cS)
{
    cS->label = controls_label;                                 // Initialized by X Macro
    cS->val = controls_val;                                     // Initialized by X Macro
    cS->focus = controls_focus;
    for(int i=0; i<NUM_CTRLS; i++) cS->focus[i] = false;        // Initialize to false
    cS->fg_rect = controls_fg_rect;
    for(int i=0; i<NUM_CTRLS; i++)
    { // Zero-init y,w,h (adjusted during rendering) -- hardcode x to dT_margin
        cS->fg_rect[i].x = dT_margin;
        cS->fg_rect[i].y = 0;
        cS->fg_rect[i].w = 0;
        cS->fg_rect[i].h = 0;
    }
    cS->bg_rect = controls_bg_rect;
    for(int i=0; i<NUM_CTRLS; i++)
    { // Zero-init y,w,h (adjusted during rendering) -- hardcode x to 0
        cS->bg_rect[i] = (SDL_Rect){0};
    }
}

void ctrl_has_focus(bool **TheFocus, Ctrl_SOA *cS)
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

#endif // __CONTROLS_H__
