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
#define NUM_CTRLS (int)(sizeof(controls_val)/sizeof(int))

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
    const char **label;                                // Text label (ex "R: ")
    char *buff_in[NUM_CTRLS];                          // Control text input buff
    char *buff_c[NUM_CTRLS];                           // Walk the input buff
    char *buff_end[NUM_CTRLS];                         // Store where buff ends
    SDL_Texture *tex[NUM_CTRLS];                       // Textures
    SDL_Rect *fg_rect;                                 // Text inner bounding box
    SDL_Rect *bg_rect;                                 // Text outer box (margins)
    bool *focus;                                       // Has user's focus cS->focus[i]
} Ctrl_SOA;

void ctrl_load_table(Ctrl_SOA *cS)
{
    { // Static mem stuff
        { // These values are set in the CTRL_TABLE
            cS->label = controls_label;                         // Initialized by X Macro
            cS->val = controls_val;                             // Initialized by X Macro
            cS->max_val = controls_max_val;                     // Initialized by X Macro
            cS->min_val = controls_min_val;                     // Initialized by X Macro
        }
        { // Don't use CTRL_TABLE for these values: unnecessary complexity
            cS->focus = controls_focus;                         //
            for(int i=0; i<NUM_CTRLS; i++) cS->focus[i] = false;// Initialize to false
            cS->fg_rect = controls_fg_rect;
            for(int i=0; i<NUM_CTRLS; i++)
            { // Zero-init y,w,h (they are later adjusted during rendering)
                cS->fg_rect[i].x = dT_margin;                   // hardcode x to dT_margin
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
            cS->text[i] = malloc(sizeof(char)*32);              // Entire ctrl: at most 32

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

void ctrl_inc(Ctrl_SOA *cS)
{
    for( int i=0; i<NUM_CTRLS; i++ )
    {
        if(  cS->focus[i]  )
        {
            cS->val[i]++;
            // Clamp at max
            if (  cS->val[i] > cS->max_val[i]  ) cS->val[i]=cS->max_val[i];
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
            // Clamp at min
            if (  cS->val[i] < cS->min_val[i]  ) cS->val[i]=cS->min_val[i];
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
            if(  cS->buff_c[i] > cS->buff_end[i]  ) cS->buff_c[i] = cS->buff_end[i];
            *cS->buff_c[i] = '\0';                  // nul-terminate string
            break;
        }
    }


}
#endif // __CONTROLS_H__
