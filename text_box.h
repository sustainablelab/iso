#ifndef __TEXT_BOX_H__
#define __TEXT_BOX_H__

#include <SDL.h>
#include "debug_text.h"

typedef struct
{
    char *text;
    SDL_Texture *tex;
    SDL_Rect fg_rect;
    SDL_Color fg;
    SDL_Rect bg_rect;
    SDL_Color bg;
    int margin;
    bool show;
    bool focus;
} TextBox;

void setup_debug_box(TextBox *tb, char *buffer)
{
    tb->text = buffer;                                          // Point at text buffer
    tb->fg = dT_normal_color;                                   // White text
    tb->bg = dT_glow;                                           // Green glow when active
    tb->margin = dT_margin;                                     // Space (pix) to win edge
    tb->fg_rect=(SDL_Rect){.x=tb->margin, .y=tb->margin,        // Left/Top edge is margin
                           .w=0, .h=0};                         // Will size text in loop
    tb->bg_rect=(SDL_Rect){0};                                  // Will size bgnd in loop
    tb->focus = false;                                          // Start: not user's focus
}

#endif // __TEXT_BOX_H__

