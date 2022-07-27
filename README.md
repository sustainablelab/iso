# UI make map

I want to easily edit the top map. I don't want to make a map
editing program. I don't want to edit the points manually in
code or in a text editor. The reason is the disconnect: I have to
figure out where I am on the map and where I am in the list of
points I'm editing. But if I do this in the running program, I
get that connection.

To avoid this getting crazy, I don't want to see all the points
in the debug overlay. My idea is to just show a single control,
then use the keyboard to navigate through points. The control
will update with the value of that point and I can then edit it.

- Make a debug control that edits points.
- Arrow keys move me between points.
    - Rectangle shows which point I'm on.
- i to insert like usual
- Enter to submit new value like usual
- Shift+Enter takes me to the next control like usual
- new stuff:
- I (Shift-i) adds a NEW point after the current point.
    - Adding points is not a big deal
    - The points polygon is created and destroyed on every
      iteration of the game loop
    - this gives me a blank field with the default value 0,0
    - the map will look goofy until I add a value

- And of course:
    - save to write this map to file
        - warn if trying to exit and map is changed
        - put warning message in main overlay area, like how I do
          the help message
        - put warning in red
        - something like "press Esc again to really exit or
          press anything else to stay in program"
    - load from file when program starts
    - load to reload from file (this way I can also edit directly
      in the text file and do a reload, or I can do a restore if
      I try a bunch of edits I don't like)


# Visual obscurity

I draw lines around a polygon. These are artistic lines to add
depth to the image. Which lines are visible? I can explain the
thought process to a human, but how do I tell the computer?

Consider the number of edge crossings. The line in question is
carved up by its intersections with the polygon.

Depending on whether the endpoints of this line are inside or
outside the polygon, I can then determine which carved bits of
the line are visible.

## Inside or outside

I want to know if a point is inside or outside a polygon. I have
come up with a novel approach for this.

The calculation goes in two steps.

A polygon is a list of points. Make a new temporary polygon that
includes the point in question.

The first step is to decide where in the polygon to insert the
new point. It must be inserted such that it does not induce a
twist in the polygon. Find the side the point is closest to and
insert it between the two points that define that side.

The second step is easy: calculate the signed area of the
original polygon and the new temporary polygon. If the areas are
the same the point lies on a side. If the original polygon area
is bigger than the new polygon area, the point lies inside the
polygon. If the original polygon area is smaller than the new
polygon area, the point lies outside the original polygon.

## Edge crossings

The depth art lines always start at vertices. Consider the vertex
to be a point outside the polygon. If the line passes through a
vertex, don't count that as an intersection.

If that line never crosses an edge and it ends in the polygon, do
not draw it. The line is completely obscured.

If the line crosses one edge and it ends in the polygon, then it
started outside the polygon -- draw the portion from the start up
to the intersection.

If the line crosses one edge and it does not end in the polygon,
then it started inside the polygon -- draw the portion from the
intersection up to the end.

If the line crosses two edges and it ends in the polygon, draw
the portion between the two intersections.

If the line crosses two edges and it does not end in the polygon,
draw everything except the portion between the two intersections.

There is an idea to generalize here involving odd/even number of
intersections and which portions of the line segment to draw.

## Old thoughts

Visual obscurity is tricky. I'm missing the test for
inside/outside. And I'm not doing the odd/even checking on the
intersections.

I want to know if my depth art line is obscured by the polygon.

I have two cases maybe: intersections vs touching a vertex.

But no matter what, there is the question of intersections.

I've been finding intersections by first considering the infinite
lines, finding their intersections, then testing if those
intersections points lie on my line segments.

## References

Maybe better ideas here:

C code: http://paulbourke.net/geometry/pointlineplane/pdb.c
Explanation: http://paulbourke.net/geometry/pointlineplane/


Other question is the inside or outside

http://paulbourke.net/geometry/polygonmesh/index.html#insidepoly

All the Norman Wildberger play lists:

Elementary Mathematics (K-6) Explained: https://www.youtube.com/playlist?
list=PL8403C2F0C89B1333
Year 9 Maths: https://www.youtube.com/playlist?list...
Ancient Mathematics: https://www.youtube.com/playlist?list...
Wild West Banking: https://www.youtube.com/playlist?list...
Sociology and Pure Mathematics: https://www.youtube.com/playlist?list...
Old Babylonian Mathematics (with Daniel Mansfield): https://www.youtube.com/playlist?
list=PLIljB45xT85CdeBmQZ2QiCEnPQn5KQ6ov
Math History: https://www.youtube.com/playlist?list...
Wild Trig: Intro to Rational Trigonometry: https://www.youtube.com/playlist?list...
MathFoundations: https://www.youtube.com/playlist?list...
Wild Linear Algebra: https://www.youtube.com/playlist?list...
Famous Math Problems: https://www.youtube.com/playlist?list...
Probability and Statistics: An Introduction: https://www.youtube.com/playlist?list...
Boole's Logic and Circuit Analysis: https://www.youtube.com/playlist?list...
Universal Hyperbolic Geometry: https://www.youtube.com/playlist?list...
Differential Geometry: https://www.youtube.com/playlist?list...
Algebraic Topology: https://www.youtube.com/playlist?list...
Math Seminars: https://www.youtube.com/playlist?list...
************************

And here are the Wild Egg Maths Playlists:

Triangle Centres: https://www.youtube.com/watch?v=iLBGX...
Six: An elementary course in pure mathematics: https://www.youtube.com/playlist?list...
Algebraic Calculus One: https://www.youtube.com/playlist?list...
Algebraic Calculus Two: https://www.youtube.com/playlist?list...
