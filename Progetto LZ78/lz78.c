#include "lz78.h"



//Display a help message
void help(){
	char* options_msg = "Usege: lz78 \n\
	 -c ..................: compressor mode\n\
	 -d ..................: decompressor mode\n\
	 -i \"<filename>\"......: input file name\n\
         -o \"<filename>\"......: output file name\n\
        [-v ].................: verbose\n\
         -s \"<size>\"..........: dictionary size \n";
        
	printf("%s", options_msg);		
}



int main(int argc, char* argv[]){
/*--------------Variables---------------*/
	char opt;
	const char* opt_string = "cdi:o:s:v";
	char* input_file = NULL;
	char* output_file = NULL;
	int dict_size = 0;
	
/*--------------Options Handle----------*/

	while((opt = getopt(argc, argv, opt_string)) != -1){
		switch(opt){
			// Option '-c' for compressor mode
			case 'c':
				
				break;
			// Option '-d' for decompressor mode
			case 'd':
				
				break;
			// Option -i <filename> for set the input file name
			case 'i':
				input_file = strdup(optarg);
				break;
			// Option -o <filename> for set the output file name
			case 'o':
				output_file = strdup(optarg);
				break;
			// Option -s <dictionary_size> for set the dictionary size
			case 's':
				dict_size = atoi(optarg);
				//------------------------------------------------------------gestire errore di size <= 0 oppure troppo grande
				break;
			// Option -v for verbose mode
			case 'v':
	
				break;
			default:
				help();
				return 0;
		}	
	}
	//-------------------------------------------------------------------------------------gestire errore opzioni obbligatorie mancanti
	printf("Input file: %s\n", input_file);
	printf("Output file: %s\n", output_file);
	printf("Dictionary size: %i\n", dict_size);
	
	/*_____________________________TESTING HASH TABLE____________________________*/
	
	hashtable_t* hashtable;
	hashtable = create_hash_table(100);
	if(hashtable == NULL){
		printf("ERROR IN HASH TABLE GENERATION\n");
		return 0;
	}
	int hashval = 0;
	hashval = (int)key_generation(hashtable, 4, 'c');
	printf("Hash value is = %i\n", hashval);
	hashval = (int)key_generation(hashtable, 2, 'a');
	printf("Hash value is = %i\n", hashval);
	hashval = (int)key_generation(hashtable, 0, 'z');
	printf("Hash value is = %i\n", hashval);
	hashval = (int)key_generation(hashtable, 15, 'h');
	printf("Hash value is = %i\n", hashval);
	hashval = (int)key_generation(hashtable, 4, 'c');
	printf("Hash value is = %i\n", hashval);
	hashval = (int)key_generation(hashtable, 3, 's');
	printf("Hash value is = %i\n", hashval);
	
	return 0;
}
