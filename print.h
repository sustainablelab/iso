#ifndef __PRINT_H__
#define __PRINT_H__

/* *************TODO***************
 * Replace sprintf with my own int to char conversion
 * *******************************/
#include <stdio.h>
// c : walk src str
// d : walk dst str
// BEWARE: this assumes the dst str is at least as long as the src str
#define print(str) { const char *c = str; while(*c!='\0'){*d++=*c++;} *d='\0'; }
#define printint(ndig,val) { char str[ndig+1]; sprintf(str, "%d", val); print(str); }

#endif // __PRINT_H__
