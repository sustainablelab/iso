#ifndef __CONTROLS_H__
#define __CONTROLS_H__

#include <stdbool.h>
#include <SDL.h>

// Add a control by adding an entry to this table. That's it.
#define CTRL_TABLE                          \
    /*    index,label, val, focus, rect, */ \
        X(R,    "R: ", 128, false, {0})     \
        X(G,    "G: ", 128, false, {0})     \
        X(B,    "B: ",   0, false, {0})     \
        X(A,    "A: ",   0, false, {0})     \
        X(X,    "X: ",   0, false, {0})     \
        X(Y,    "Y: ",   0, false, {0})

// Preprocessor output: _Bool ctrl_focus[] = { 0 , 0 , 0 , 0 , };
#define X(index, label, val, focus, rect) focus,
bool ctrl_focus[] = { CTRL_TABLE };
#undef X

#define NUM_CTRLS (int)(sizeof(ctrl_focus)/sizeof(bool))

// Preprocessor output: enum ctrl_index { R, G, B, A, };
#define X(index, label, val, focus, rect) index,
enum ctrl_index { CTRL_TABLE };
#undef X

// Preprocessor output: const char *ctrl_label[] = { "R: ", "G: ", "B: ", "A: ", };
#define X(index, label, val, focus, rect) label,
const char *ctrl_label[] = { CTRL_TABLE };
#undef X

// Preprocessor output: int ctrl_val[] = { 128, 128, 0, 0, };
#define X(index, label, val, focus, rect) val,
int ctrl_val[] = { CTRL_TABLE };
#undef X

// Zero-initialize x,y,w,h for the fgnd and bgnd text rectangles
#define X(index, label, val, focus, rect) rect,
SDL_Rect ctrl_fg_rect[] = { CTRL_TABLE };
SDL_Rect ctrl_bg_rect[] = { CTRL_TABLE };
#undef X


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
    SDL_Rect *fg_rect;                                          // Text bounding box
    SDL_Rect *bg_rect;                                          // Text box with margins
    bool *focus;                                                // Has user's focus cS->focus[i]
} Ctrl_SOA;

void ctrl_load_table(Ctrl_SOA *cS)
{
    cS->label = ctrl_label;
    cS->val = ctrl_val;
    cS->focus = ctrl_focus;
    cS->fg_rect = ctrl_fg_rect;
    cS->bg_rect = ctrl_bg_rect;
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
