#ifndef __LINE_H__
#define __LINE_H__

#include <SDL.h>
#include "vec.h"
typedef struct
{
    int x1,y1,x2,y2;
} Line;

void line_move(Line *l, Vec2 offset)
{
    l->x1 += offset.x;
    l->y1 += offset.y;
    l->x2 += offset.x;
    l->y2 += offset.y;
}

void line_draw(SDL_Renderer *ren, Line l)
{
    SDL_RenderDrawLine(ren, l.x1, l.y1, l.x2, l.y2);
}

void line_map_top_to_iso(Line *l)
{ // Change l from top coord to iso coord
    // Map line points from top-view to iso-view
    Vec2 p1 = {l->x1, l->y1}; vec_map_top_to_iso(&p1);
    Vec2 p2 = {l->x2, l->y2}; vec_map_top_to_iso(&p2);
    // Write mapped points back to line
    l->x1 = p1.x; l->y1 = p1.y;
    l->x2 = p2.x; l->y2 = p2.y;
}

#endif // __LINE_H__
