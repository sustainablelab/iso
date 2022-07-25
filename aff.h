#ifndef __AFF_H__
#define __AFF_H__

/* *************Affine geometry***************
 * Series: WildLinAlg: A geometric course in Linear Algebra
 *
 * - WildLinAlg10: Equations of lines and planes in 3D
 *
 * Series: WildTrig: Intro to Rational Trigonometry
 *
 * - WildTrig12: Cartesian coordinates and geometry
 * - WildTrig42: An algebraic framework for rational trigonometry (I)
 *
 * Series: Wild Egg Maths Algebraic Calculus One: 
 *
 * - Points and Lines in the Affine Plane
 * - Projective Coordinates for Points and Lines
 *
 * *******************************/
/* *************Types***************
 * Object   :   Notation            :   Interpretation
 * ------   :   --------            :   --------------
 * AffPoint :   { x1, y1 }          :
 *          :                       :
 * AffLine  :   { a, b, c }         :   ax + by = c
 *          :                       :   - "infinite" length
 *          :                       :   - fixed origin (has notion of colinearity)
 *          :                       :   - no direction
 *          :                       :
 * AffVec   :   {   α  ,   β   }    :   vector vaB = (αx, βy)
 *          :   { x2-x1, y2-y1 }    :   - finite length
 *          :                       :   - no fixed origin (no notion of colinearity)
 *          :                       :   - has direction
 *          :                       :
 * AffSeg   :   {A{x1,y1}, B{x2,y2}}:   directed line segment sAB is a line from A to B
 *          :                       :   - finite length
 *          :                       :   - fixed position (A and B are colinear with sAB)
 *          :                       :   - has direction:
 *          :                       :       - direction from A to B is { x2-x1, y2-y1 }
 *          :                       :       - adding this α,β to x1,y1 yields x2,y2
 *          :                       :
 *
 * *******************************/
/* *************Definitions***************
 * AffPoint A lies on AffLine l         <=> a*x1 + b*y1 = c
 * AffLines l1 and l2 are parallel      <=> a1*b2 - a2*b1 = 0
 * AffLines l1 and l2 are perpendicular <=> a1*a2 + b1*b2 = 0
 * JoinOfPoints 1 (AffVec)  : AffPoints A and B form directed line segment sAB
 * JoinOfPoints 2 (AffLine) : AffPoints A and B define line l
 * *******************************/
/* *************Recipes***************
 * 1. Find AffLine l given AffPoints A and B
 *
 * Inputs
 * ------
 * AffPoint A = {x1,y1}
 * AffPoint B = {x2,y2}
 *
 * Intermediate Values
 * -------------------
 * AffVec vAB = {x2-x1,y2-y1}
 *
 * Output
 * ------
 * AffLine l = {-β, α, c}
 * β = y2-y1
 * α = x2-x1
 * c = -β*x1 + α*y1
 *
 * Closed Form
 * -----------
 * Affline l = {-1*(y2-y1), (x2-x1), -1*(y2-y1)*x1 + (x2-x1)*y1}
 * *******************************/

// Aliases to use math terminology with SDL API
typedef SDL_FPoint AffPoint;
typedef AffPoint AffVec;

typedef struct
{
    AffPoint A, B;
} AffSeg;

typedef struct
{
    float a,b,c;
} AffLine;

AffLine aff_join_of_points(AffPoint A, AffPoint B)
{ // Return join of points A and B
    float alpha = B.x-A.x; float beta = B.y-A.y;
    float c = -1*beta*A.x + alpha*A.y;
    AffLine l = {-1*beta, alpha, c};
    return l;
}

AffPoint aff_meet_of_lines(AffLine l1, AffLine l2)
{ // Return meet of lines l1 and l2
    float a1 = l1.a; float b1 = l1.b; float c1 = l1.c;
    float a2 = l2.a; float b2 = l2.b; float c2 = l2.c;
    float det = 1/(a1*b2 - a2*b1);
    float x = det*(b2*c1 - b1*c2);
    float y = det*(a1*c2 - a2*c1);
    AffPoint M = {x,y};
    return M;
}

AffVec aff_vec_from_points(AffPoint A, AffPoint B)
{ // Return vector AB (the vector that goes from A to B)
    return (AffPoint){B.x-A.x, B.y-A.y};
}

bool aff_point_on_seg(AffPoint M, AffSeg seg)
{
    // First establish that lines AB and AM are parallel
    AffLine line_AB = aff_join_of_points(seg.A, seg.B);
    AffLine line_AM = aff_join_of_points(seg.A, M);
    float a1 = line_AB.a; float b1 = line_AB.b;
    float a2 = line_AM.a; float b2 = line_AM.b;
    if(  (a1*b2 - a2*b1)!=0  ) { return false; }
    // So they are parallel -- now check the scaling factor lambda for the two vectors
    AffVec vec_AB = aff_vec_from_points(seg.A, seg.B);
    AffVec vec_AM = aff_vec_from_points(seg.A, M);
    if(  (vec_AB.x == 0) && (vec_AB.y == 0)  ) // vec.x and vec.y cannot both be zero!
    { // Both vector components are zero -- it is meaningless to ask if point is on seg
        return false;
    }
    // Find lambda (the scaling between the two vectors)
    float lambda;
    if(  vec_AB.x != 0  )   { lambda = vec_AM.x / vec_AB.x; }   // Use vec.x if non-zero
    else                    { lambda = vec_AM.y / vec_AB.y; }   // Use vec.y if vec.x is 0
    if(  lambda < 0  ) {lambda = -1*lambda;}                    // Make sure lambda is positive
    if(  lambda > 1  ) { return false;}                         // lambda > 1 -- point is off line seg
    else { return true; }                                       // lambda <=1 -- point is on line seg
}
#endif // __AFF_H__

