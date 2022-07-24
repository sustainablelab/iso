#ifndef __VEC_H__
#define __VEC_H__

typedef struct
{
    int x,y;
} Vec2;

void vec_map_top_to_iso(Vec2 *p)
{ // Change p from top coord to iso coord
    /* *************DOC***************
     * Example:
     * If top coord are 2,3, expect iso coord are -2,5
     * x :  x-y = -1
     *      2*-1 = -2 <-- yes
     * y :  x+y = 5 <---- yes
     * *******************************/
    Vec2 top = {p->x, p->y};
    p->x = 2*(top.x - top.y);
    p->y =   (top.x + top.y);
}
#endif // __VEC_H__
