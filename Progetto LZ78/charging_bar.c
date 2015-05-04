#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>


/*
 * 
 * name: loadBar
 * @param
 * 		Process has done x out of n rounds,
 * 		and we want a bar of width w and resolution r.
 * @return
 * 
 */
void loadBar(int x, int n, int r, int w)
{
	int i, j;
	float ratio;
	int c;
    // Only update r times.
    if ( x % (n/r +1) != 0 ) return;
 
    // Calculuate the ratio of complete-to-incomplete.
    ratio = x/(float)n;
    c = ratio * w;
 
    // Show the percentage complete.
    printf("%3d%% [", (int)(ratio*100) );
 
    // Show the load bar.
    for (i=0; i<c; i++)
       printf("=");
 
    for (j=c; j<w; j++)
       printf(" ");
 
    // ANSI Control codes to go back to the
    // previous line and clear it.
    printf("]\n\033[F\033[J");
}


int main(){
	int i = 0;
	char output[10];
	printf("test charging bar: \n");
	
	do{
		loadBar(i, 40, 40, 40);
		sleep(1);
		i++;
	}while(i<40);
	return 0;
}
