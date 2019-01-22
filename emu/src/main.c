#include "nb.h"
/*
TODO	
	- Create (Tracing) Mark-Sweep Garbage collection system.
	Pointer struct
	See http://libcello.org/learn/garbage-collection
	- Create QuadTree for sprites. Use for rendering
	- Create "Batched" Rendering for multi-sheet levels?
	- Render targets. Deferred/Forward Rendering!
*/

extern void demo();

void show_mem_rep(char *start, int n)  
{ 
    int i; 
    for (i = 0; i < n; i++) 
         printf(" %.2x", start[i]); 
    printf("\n"); 
} 
  
int main()
{
    demo();
 
}
