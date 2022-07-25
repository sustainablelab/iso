                if ( 0 )
                { // A failed first attempt at visual obscurity
                    //If I do figure out how to do it this way, remember to free(has_hatch)
                    /* free(has_hatch); */

                    /* *************DOC***************
                     * Argh, the algorithm below works, but
                     * it only catches points directly above other points
                     * That is NOT what I need to test for....
                     * *******************************/

                    // Decide who gets a hatch:
                    // If multiple points have same x/y ratio, only biggest point gets hatch line
                    bool *has_hatch = malloc(cnt*sizeof(bool)); // Who has a hatch
                    for( int i=0; i<(cnt-1); i++) { has_hatch[i] = true; }
                    for( int i=0; i<(cnt-1); i++)               // -1 because end point is start point
                    {
                        int a = points[i].x, b = points[i].y;   // This point
                        // Turn 0,0 into 1,1, just for this calculation
                        if(  (a == 0) && (b == 0)  ) { a = 1; b = 1; }
                        for( int j=0; j<(cnt-1); j++)
                        { // Compare this point with all others
                            if(  i!=j  )                        // Don't compare point with itself
                            {
                                int c = points[j].x, d = points[j].y;// Another point
                                // Turn 0,0 into 1,1, just for this calculation
                                if(  (c==0) && (d==0)  ) { c = 1; d = 1; }
                                if(  (a*d) == (b*c)  )
                                { // These points are on the same vertical!
                                    if(a<c)
                                    {
                                        has_hatch[i] = false;   // Do not hatch this point
                                        break;                  // Done checking this point
                                    }
                                }
                            }
                        }
                    }
                    has_hatch[(cnt-1)] = has_hatch[0];          // because end point is start point
                }

                    if(0)
                    { // Old hacky way
                        int len = 10; // cS->val[S]*3;
                        for( int i=0; i<cnt; i++)
                        {
                            if(  has_hatch[i]  )
                            {
                                int x = points[i].x; int y = points[i].y;
                                Line l = {x, y, x, y+len};
                                line_draw(ren, l);
                            }
                        }
                    }
