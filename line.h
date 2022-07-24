#ifndef __LINE_H__
#define __LINE_H__

#include <SDL.h>
#include "vec.h"

/* *************DOC***************
 * Decision: use lines for hatch shading. Art stuff. Not for the game map.
 * Use points for the game map.
 *
 * As art stuff, I can:
 *
 * - draw directly in the iso view (e.g., a vertical line)
 * - draw in the top view (e.g., draw a 45° line)
 *      - then map that line to the iso view (it becomes a vertical line)
 * *******************************/
/* *************Example using lines***************
 * This example draws a game map in the top view using lines,
 * then I map that to the iso view and draw it again.
 * This example does everything with lines, no points.
 *
 * // Draw top-down view
 * int s = cS->val[S];
 * Line v1 = {0,0,0,100};                              // Vertical line
 * Line v2 = {s,s,s,100};                           // Vertical line
 * Line h1 = {0,0,100,0};                              // Horizontal line
 * Line h2 = {s,s,100,s};                           // Horizontal line
 * // Draw original in ghostly shade on the side
 * SDL_SetRenderDrawColor(ren, 150,60,140,50);         // Line color for ghost
 * Line tv1 = v1; Line th1 = h1;                       // Is this a copy? Yes!
 * Line tv2 = v2; Line th2 = h2;                       // Is this a copy? Yes!
 * Vec2 toffset = {50, cS->val[Y1]};                   // Translate origin 0,0
 * line_move(&tv1, toffset); line_move(&th1, toffset);
 * line_move(&tv2, toffset); line_move(&th2, toffset);
 * line_draw(ren, tv1); line_draw(ren, th1);
 * line_draw(ren, tv2); line_draw(ren, th2);
 * // Map
 * line_map_top_to_iso(&v1); line_map_top_to_iso(&h1); // Map coord system
 * line_map_top_to_iso(&v2); line_map_top_to_iso(&h2); // Map coord system
 * Vec2 offset = {cS->val[X1], cS->val[Y1]};           // Translate origin 0,0
 * line_move(&v1, offset); line_move(&h1, offset);     // Move in screen coord
 * line_move(&v2, offset); line_move(&h2, offset);     // Move in screen coord
 * // Render
 * SDL_SetRenderDrawColor(ren, cS->val[R],cS->val[G],cS->val[B],cS->val[A]);
 * line_draw(ren, v1); line_draw(ren, h1);
 * line_draw(ren, v2); line_draw(ren, h2);
 * *******************************/

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
