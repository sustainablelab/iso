#ifndef __POINT_H__
#define __POINT_H__

#include <SDL.h>

void point_move(SDL_Point *p, SDL_Point offset)
{ // Add the offset to the point -- int version
    p->x += offset.x;
    p->y += offset.y;
}
void point_fmove(SDL_FPoint *p, SDL_FPoint offset)
{ // Add the offset to the point -- float version
    p->x += offset.x;
    p->y += offset.y;
}

void point_map_top_to_iso(SDL_Point *p)
{ // Change p from top coord to iso coord -- int version
    /* *************DOC***************
     * Example:
     * If top coord are 2,3, expect iso coord are -2,5
     * x :  x-y = -1
     *      2*-1 = -2 <-- yes
     * y :  x+y = 5 <---- yes
     * *******************************/
    SDL_Point top = {p->x, p->y};
    p->x = 2*(top.x - top.y);
    p->y =   (top.x + top.y);
}
void point_fmap_top_to_iso(SDL_FPoint *p)
{ // Change p from top coord to iso coord -- float version
    /* *************DOC***************
     * See int version
     * *******************************/
    SDL_FPoint top = {p->x, p->y};
    p->x = 2*(top.x - top.y);
    p->y =   (top.x + top.y);
}

#endif // __POINT_H__
