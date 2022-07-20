#ifndef __TEXT_BOX_H__
#define __TEXT_BOX_H__

#include <SDL.h>
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

#endif // __TEXT_BOX_H__

