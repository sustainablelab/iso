#ifndef __POINT_H__
#define __POINT_H__

#include <SDL.h>

void point_move(SDL_Point *p, SDL_Point offset)
{
    p->x += offset.x;
    p->y += offset.y;
}

void point_map_top_to_iso(SDL_Point *p)
{ // Change p from top coord to iso coord
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

#endif // __POINT_H__
