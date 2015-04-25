#include "lz78.h"

struct node{
	int father_id;
	char symbol;
};


int compute_bit_length(int num){
	int bits = 8;
	int bound = 256;
	//max encoding bit dimention = 32.
	while(bits < 32){
		if(num < bound)
			return bits;
		else{
			bound *= 2;
			bits++;
		}
	}
	return 32;
}


/*Reads the encoding of the tree node from the file
* Input param: File descriptor, dimension of the tree (needed for know how many bits it have to read)
* Return: -1 if an error occurs, otherwise the node identifier
*/
int read_node(struct bitio* fd, int tree_dim){
	uint64_t node_id;
	int how_many; 
	int ret;
	if(fd == NULL || tree_dim < 0){
		return -1;
	}
	//compute the number of bits to read with logarithm in base 2
	how_many = compute_bit_length(tree_dim);
	
	//read the bits from the file 
	ret = bitio_read_chunk(fd, &node_id, how_many);
	if(ret != how_many){
		return -1;
	}
	printf("I have read %i on %i bits\n", (int)node_id, how_many);
	return (int) node_id;
}


void retrieve_string(struct node* tree, int node_id, char* partial_string, char* inverse_string, int *size_array){
	int i;
	int counter = 0;
	int previous_id = node_id;
	
	//reset strings
	memset(partial_string, 0, *size_array * sizeof(char));
	memset(inverse_string, 0, *size_array * sizeof(char));
	
	printf("Risalgo l'albero dal nodo %i\n", node_id);
	while(1){
		if(counter == (*size_array)-1){
			*size_array = *size_array + 1;
			partial_string = (char*)realloc(partial_string, *size_array * sizeof(char));
			inverse_string = (char*)realloc(inverse_string, *size_array * sizeof(char));
			memset(partial_string, 0, *size_array * sizeof(char));
			memset(inverse_string, 0, *size_array * sizeof(char));
			previous_id = node_id;
			counter = 0;
		}
		if(previous_id <= 255)
			inverse_string[counter] = (char)(tree[previous_id].symbol - 1);
		else
			inverse_string[counter] = (char)(tree[previous_id].symbol);
		previous_id = tree[previous_id].father_id;
		printf("Symbolo = %c; Padre = %i\n",inverse_string[counter], previous_id);
		if(previous_id == 0 ){
			break;
		}
		counter++;
	}
	//invert string
	for(i = counter ; i >= 0 ; i--){
		partial_string[counter-i] = inverse_string[i];
	}
}

void init_tree(struct node* tree){
	char ascii;
	int i;
	for(i = 0; i <= 255; i++){
		ascii = (char)i;
		tree[i].father_id = 0;
		tree[i].symbol = ascii;
	} 
	return;
}



int decompressor(char* input_file, char* output_file, int verbose_mode){
	struct bitio* input;
	FILE* output;
	struct node* tree;
	int tree_max_size = 256;
	int tree_size = 257;	//First id starts from 257
	int node_id;
	int old_node_id; 
	int size_array = 50;	//default value
	char* partial_string = (char*)malloc( size_array * sizeof(char) );
	char* inverse_string = (char*)malloc( size_array * sizeof(char) );
	
	//Open the compressed file
	input = bitio_open(input_file, 'r');
	if(input==NULL){
		printf("Error with input file. You are sure that exists?\n");
		return -1;
	}
	//Open the (decompressed) output file
	output = fopen(output_file, "w");	
	if(output<0){
		printf("Impossible to open output file.\n");
		return -1;
	}
	//Allocate the tree structure
	tree_max_size += 300; 						//dictionary size = 300 per ora, poi da leggere dall'header
	tree = (struct node*)calloc(tree_size, sizeof(struct node));
	//Initialization the firt layer of the tree
	init_tree(tree);
	old_node_id = -1;
	//Decompress
	do{
		node_id = read_node(input, tree_size);
		//If the node is 0 we are at the end of the file
		if(node_id == 0)
			break;
		//Extract the string from the tree 
		retrieve_string(tree, node_id, partial_string, inverse_string, &size_array);
		fwrite (partial_string , sizeof(char), strlen(partial_string), output);
		//Update the symbol of the old entry 
		if(old_node_id > 0){					//This check is needed for the first cycle, where there are no entry to update.
			tree[old_node_id].symbol = partial_string[0];
			printf("Aggiorno Old Node = %i con symbol = %c, padre id = %i\n", old_node_id, tree[old_node_id].symbol, tree[old_node_id].father_id);
		}
		//Add new entry to the tree
		tree[tree_size].father_id = node_id;
		old_node_id = tree_size;
		tree_size++;
		//Tree is full, reset it!
		if(tree_size == tree_max_size){
			tree_size = 256;
			old_node_id = -1;
		}
	}
	while(1);
	
	fclose(output);
	bitio_close(input);
	//free(partial_string);				//Danno errore...come mai?
	//free(inverse_string);
	//free(tree);
	return 1;
	
}
