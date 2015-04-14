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
	int node_id = -1;
	int find;
	insert(hashtable, 'a', 0);
	insert(hashtable, 'b', 0);
	insert(hashtable, 'c', 0);
	insert(hashtable, 'd', 0);
	
	node_id = (int)search(hashtable, 'a', 0, &find);
	if(find == 1)
		printf("The node was found, node id = %i\n", node_id); 	
	else
		printf("The node wasn't found, node id = %i\n", node_id); 
	
	node_id = (int)search(hashtable, 'c', 0, &find);
	if(find == 1)
		printf("The node was found, node id = %i\n", node_id); 	
	else
		printf("The node wasn't found, node id = %i\n", node_id);
		
	node_id = insert(hashtable, 'a', node_id);
	node_id = insert(hashtable, 's', node_id);
	
	node_id = (int)search(hashtable, 'c', 0, &find);
	if(find == 1)
		printf("The node was found, node id = %i\n", node_id); 	
	else
		printf("The node wasn't found, node id = %i\n", node_id); 
	
	node_id = (int)search(hashtable, 'a', node_id, &find);
	if(find == 1)
		printf("The node was found, node id = %i\n", node_id); 	
	else
		printf("The node wasn't found, node id = %i\n", node_id);
	
	node_id = (int)search(hashtable, 'x', node_id, &find);
	if(find == 1)
		printf("The node was found, node id = %i\n", node_id); 	
	else
		printf("The node wasn't found, node id = %i\n", node_id);
	
	return 0;
}
