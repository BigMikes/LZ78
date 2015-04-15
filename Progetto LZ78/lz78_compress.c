#include "lz78.h"

int compressor(char* input_file, char* output_file, int dictionary_size, int verbose){
	/*------------variabili---------------------------------------------------*/
	int input;
	struct bitio* output;
	hashtable_t* hashtable;
	int i;
	char ascii;
	/*------------programma---------------------------------------------------*/
	
	input = open(input_file, O_RDONLY);
	if(input<0){
		printv(verbose, "Impossible to open input file. You are sure taht exists?\n");
		return -1;
	}
	output = bit_open(output_file, 'w');
	if(output==NULL){
		printv(verbose, "Error with output file\n");
		return -1;
	}
	hashtable create_hash_table(dictionary_size);
	if(hashtable == NULL){
		printv(verbose, "ERROR IN HASH TABLE GENERATION\n");
		return -1;
	}
	
	for(i = 0 ; i <= 255 ; i++){
		ascii=i;
		printv(verbose, );
		insert(hashtable, ascii, 0);
	}
	
}
