#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>


static inline void loadBar(int x, int n, int r, int w)
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
		/*fflush(stdout);
		output[i]='-';
		printf("%s  %i%%", output, (i+1)*10);*/
		loadBar(i, 10, 20, 20);
		sleep(1);
		i++;
	}while(i<10);
	printf("\n");
	return 0;
}
