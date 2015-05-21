#include "lz78.h"

struct parameters{
	//mode 1->compressor
	//mode 2->decompressor
	int mode; 
	char* input_file;
	char* output_file;
	//verbose is active if 1
	int verbose;
	int dict_size;
	int i_set;
	int o_set;
	int s_set;
};




/* 		Process has done x out of n rounds,
 * 		and we want a bar of width w and resolution 
 */
inline void loadBar(int verbose, int x, int n, int r, int w){
	int i, j;
	float ratio;
	int c;
	
	if(n == 0)
		return;
        // Only update r times.
        if ( x % (n/r +1) != 0 ) return;
 
        // Calculuate the ratio of complete-to-incomplete.
        ratio = x/(float)n;
        c = ratio * w;
 
        // Show the percentage complete.
        printv(verbose, "%3d%% [", (int)(ratio*100) );
 
        // Show the load bar.
        for (i=0; i<c; i++)
        	printv(verbose, "=");
 
        for (j=c; j<w; j++)
       		printv(verbose, " ");
 
        // ANSI Control codes to go back to the
        // previous line and clear it.
        printv(verbose, "]\n\033[F\033[J");
}

//verbose print
void printv(int verbosity, const char *format, ...){
    va_list args;
    va_start(args, format);

    if (verbosity){
        vfprintf (stdout, format, args);
	}
	va_end(args);
}

//Display a help message
void help(){
	char* options_msg = "Usage: lz78 \n\
		-c ....................: compressor mode\n\
		-d ....................: decompressor mode\n\
		-i \"<filename>\"........: input file name\n\
		-o \"<filename>\"........: output file name\n\
		-v ....................: verbose\n\
		-s \"<size>\"............: dictionary size \n\
		-h ....................: this helper\n";
        
	printf("%s", options_msg);		
}




int main(int argc, char* argv[]){
/*--------------Variables---------------------------------------------*/
	char opt;
	struct parameters param;
	int ret;
	char response;

/*--------------Inizilize default parameters struct-------------------*/
	param.mode = 0;
	param.input_file = NULL;
	param.output_file = NULL;
	param.verbose = 0;
	param.dict_size = 0;
	param.i_set = 0;
	param.o_set = 0;
	param.s_set = 0;

/*--------------Options Handle----------------------------------------*/
	if(argc==1){
		help();
		return 0;
	}
	while((opt = getopt(argc, argv, "cdi:o:s:vh")) != -1){
		switch(opt){
			// Option '-c' for compressor mode
			case 'c':
				if(param.mode!=0){
					printf("It's not possible to code and decode");
					return 0;
				}
				param.mode = 1;
				break;
			// Option '-d' for decompressor mode
			case 'd':
				if(param.mode!=0){
					printf("It's not possible to code and decode");
					return 0;
				}
				param.mode = 2;
				break;
			// Option -i <filename> for set the input file name
			case 'i':
				param.i_set = 1;
				param.input_file = strdup(optarg);
				break;
			// Option -o <filename> for set the output file name
			case 'o':
				param.o_set = 1;
				param.output_file = strdup(optarg);
				break;
			// Option -s <dictionary_size> for set the dictionary size
			case 's':
				param.s_set = 1;
				param.dict_size = atoi(optarg);
				break;
			// Option -v for verbose mode
			case 'v':
				param.verbose = 1;
				break;
			case 'h':
			default:
				help();
				return 0;
		}	
	}

/*-------------gestione errori----------------------------------------*/
	//file di input necessario
	//il file è sempre accompagnato dall'estensione per cui è sempre di almeno 5 caratteri
	if(!param.i_set){	
		printf("needed input file\n");
		return 0;
	}
	
	//se il file di output non viene fornito di default è output.txt
	if(!param.o_set){
		param.output_file = (char*)malloc(sizeof("output.txt")+1);
		param.output_file = "output.txt";
	}
	
	//controllo sulla dimensione
	if(!param.s_set){
		param.dict_size = (int)UINT16_MAX;
	} else {
		if(param.dict_size <= MIN_DIM_DICT){						//non penso possa capitare un numero upperbound????
			printf("Out of bound value for dictionary\n");
			return 0;
		}
	}
	
	//controllo del modo di funzionamento
	if(param.mode==0){
		printf("Must be selected one functioning mode\n");
		return 0;
	}
	
	//se in modo decompressore s viene semplicemente ignorato
	
/*--------confirmation--------------------------------------------------------*/	
	
	printf("Input file: %s\n", param.input_file);
	if(param.mode==1){
		printf("Output file: %s\n", param.output_file);
		printf("Dictionary size: %i\n", param.dict_size);
	}
	printf("correct: y/n?\n");
	ret = scanf("%c", &response);
	switch(response){
		case 'y':
		case 'Y':
			if(param.mode==1){	//compressore
				printf("Start encoding\n");
			} else {
				printf("Start decoding\n");
			}
			break;
		case 'n':
		case 'N':
		default:
			printf("Abort\n");
			return 0;
	}
	
/*---------functioning--------------------------------------------------------*/
	if(param.mode==1)
		ret = compressor(param.input_file, param.output_file, param.dict_size, param.verbose);
	else
		ret = decompressor(param.input_file, param.verbose);
	
	if(ret<0){
		return 0;
	}
	if(param.mode==1){	//compressore
		printf("Encoding done\n");
	} else {
		printf("Decoding done\n");
	}
	//se cerchi il codice di test della hash table te l'ho spostato in testhashtable.c
	return 0;
}
