#ifndef __CONTROLS_H__
#define __CONTROLS_H__

#define CTRL_TABLE                      \
    /*    index,label, val, focus  */   \
        X(R,    "R: ", 128, false)      \
        X(G,    "G: ", 128, false)      \
        X(B,    "B: ",   0, false)      \
        X(A,    "A: ",   0, false)

#define X(index, label, val, focus) index,
enum ctrl_index { CTRL_TABLE };
#undef X

#define X(index, label, val, focus) label,
const char *ctrl_label[] = { CTRL_TABLE };
#undef X

#define X(index, label, val, focus) val,
int ctrl_val[] = { CTRL_TABLE };
#undef X

#define X(index, label, val, focus) focus,
bool ctrl_focus[] = { CTRL_TABLE };
#undef X

#define NUM_CTRLS 4

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
    SDL_Rect fg_rect[NUM_CTRLS];                                // Text bounding box
    SDL_Rect bg_rect[NUM_CTRLS];                                // Text box with margins
    bool *focus;                                                // Has user's focus cS->focus[i]
} Ctrl_SOA;

void ctrl_load_table(Ctrl_SOA *cS)
{
    cS->label = ctrl_label;
    cS->val = ctrl_val;
    cS->focus = ctrl_focus;
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
