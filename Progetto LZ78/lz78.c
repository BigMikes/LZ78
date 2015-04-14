#include "lz78.h"

struct parameters{
	//mode 1->compressor
	//mode 2->decompressor
	int mode; 
	char* input_file;
	char* output_file;
	int verbose;
	int dict_size;
	int i_set;
	int o_set;
	int s_set;
};

//Display a help message
void help(){
	char* options_msg = "Usage: lz78 \n\
		-c ....................: compressor mode\n\
		-d ....................: decompressor mode\n\
		-i \"<filename>\"........: input file name\n\
		-o \"<filename>\"........: output file name\n\
		-v ....................: verbose\n\
		-s \"<size>\"............: dictionary size \n\
		-h ....................: this helper";
        
	printf("%s", options_msg);		
}



int main(int argc, char* argv[]){
/*--------------Variables---------------------------------------------*/
	char opt;
	struct parameters param;

/*--------------Inizilize default parameters struct-------------------*/
	param.mode = 0;
	param.verbose = 0;
	param.dict_size = 0;
	param.i_set = 0;
	param.o_set = 0;
	param.s_set = 0;

/*--------------Options Handle----------------------------------------*/
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
	if(!param.i_set || strlen(param.input_file)==2){	
		printf("needed input file\n");
		return 0;
	}
	
	//se il file di output non viene fornito di default è output.txt
	if(!param.o_set || strlen(param.output_file)==2){
		param.output_file = (char*)malloc(sizeof("output.txt")+1);
		param.output_file = "output.txt";
	}
	
	//controllo sulla dimensione
	if(!param.s_set){
		param.dict_size = (int)UINT16_MAX;
	} else {
		if(param.dict_size<0){						//non penso possa capitare un numero upperbound????
			printf("out of bound value\n");
		}
	}
	
	//controllo del modo di funzionamento
	if(param.mode==0){
		printf("Must be selected one functioning mode");
		return 0;
	}
	
	//se in modo decompressore s viene semplicemente ignorato
/*--------------------------------------------------------------------*/	
	
	printf("Input file: %s\n", param.input_file);
	printf("Output file: %s\n", param.output_file);
	printf("Dictionary size: %i\n", param.dict_size);
	
	/*_____________________________TESTING HASH TABLE____________________________*/
	/*
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
	*/
	return 0;
}
