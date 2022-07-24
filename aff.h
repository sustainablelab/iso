#ifndef __AFF_H__
#define __AFF_H__

/* *************Affine geometry***************
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
 *          :                       :   inifinte length
 *          :                       :   fixed origin (has notion of colinearity)
 *          :                       :   no direction
 *          :                       :
 * AffVec   :   { α, β }            :   vector vaB = (αx, βy)
 *          :                       :   finite length
 *          :                       :   no fixed origin (no notion of colinearity)
 *          :                       :   has direction
 *          :                       :
 * AffSeg   :   { x2-x1, y2-y1 }    :   directed line segment sAB is a line from A to B
 *          :                       :   finite length
 *          :                       :   fixed position (A and B are colinear with vAB)
 *          :                       :   has direction
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
typedef SDL_Point AffPoint
typedef AffPoint AffVec
typedef AffPoint AffSeg

#endif // __AFF_H__

