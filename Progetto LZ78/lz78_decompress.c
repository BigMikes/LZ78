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
int read_node(int verbose_mode, struct bitio* fd, int tree_dim){
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
	printv(verbose_mode, "I have read %i on %i bits, tree size = %i\n", (int)node_id, how_many, tree_dim);
	return (int) node_id;
}


/*
* Function that given a node id, goes to see the first char of the reconstructed string
*/
char reconstruct_string(struct node* tree, int previous_id){
	int node_id = previous_id;
	char ret = 0;
	while(node_id != 0){
		ret = (char)(tree[node_id].symbol - 1);			//-1 perchè il carattere che cerchiamo è del primo layer, e questi sono scalati di 1
		node_id = tree[node_id].father_id;
	}
	return ret;
}


void retrieve_string(int verbose_mode, struct node* tree, int node_id, char* partial_string, int *size_array, int *string_len){
	int counter = 0;
	int previous_id = node_id;
	
	//reset strings
	memset(partial_string, 0, *size_array * sizeof(char));
	
	printv(verbose_mode, "Risalgo l'albero dal nodo %i\n", node_id);
	while(1){
		//Il primo layer composto da tutti i caratteri ascii è scalato di 1 rispetto all'identificatore del nodo
		if(previous_id <= 255)
			partial_string[*size_array - counter - 1] = tree[previous_id - 1].symbol;
		else{	
			//se è 0 vuol dire che siamo in quel caso particolare, dove la stringa che andiamo a ricomporre è composta da rami non ancora aggiornati
			//quindi in principio dell'algoritmo, su questi rami particolari, va il primo carattere della stringa che stiamo ricostruendo
			if(tree[previous_id].symbol == 0)					
				partial_string[*size_array - counter - 1] = reconstruct_string(tree, previous_id);
			else
				partial_string[*size_array - counter - 1] = tree[previous_id].symbol;
		}
		previous_id = tree[previous_id].father_id;
		
		if(previous_id == 0 ){
			break;
		}
		counter++;
	}
	*string_len = counter+1;
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


void clear_tree(struct node* tree, int tree_max_size){
	int i;
	for(i = 256; i <= tree_max_size; i++){
		tree[i].father_id = 0;
		tree[i].symbol = 0;
	} 
	return;
}


int decompressor(char* input_file, char* output_file, int dictionary_size, int verbose_mode){
	struct bitio* input;
	FILE* output;
	struct node* tree;
	int tree_max_size = 256;
	int tree_size = 256;	//First id starts from 257
	int node_id;
	int old_node_id; 
	char* partial_string;
	int string_len;
	
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
	tree_max_size += dictionary_size; 						//dictionary size = 300 per ora, poi da leggere dall'header
	tree = (struct node*)calloc(tree_max_size, sizeof(struct node));
	partial_string = (char*)malloc( tree_max_size * sizeof(char) );
	//Initialization the firt layer of the tree
	init_tree(tree);
	old_node_id = -1;
	//Decompress
	do{
		node_id = read_node(verbose_mode, input, tree_size);
		//If the node is 0 we are at the end of the file
		if(node_id == 0)
			break;
		//Extract the string from the tree 
		retrieve_string(verbose_mode, tree, node_id, partial_string, &tree_max_size, &string_len);
		printv(verbose_mode, "Stampo stringa = %s\n", partial_string + (tree_max_size - string_len));
		fwrite (partial_string + (tree_max_size - string_len), sizeof(char), string_len, output);
		//Update the symbol of the old entry 
		if(old_node_id > 0){					//This check is needed for the first cycle, where there are no entry to update.
			tree[old_node_id].symbol = partial_string[tree_max_size - string_len];
			printv(verbose_mode, "Aggiorno Old Node = %i con symbol = %c, padre id = %i\n", old_node_id, tree[old_node_id].symbol, 				tree[old_node_id].father_id);
		}
		tree_size++;						//As in the compressor, increments first, then uses the id
		//Add new entry to the tree
		tree[tree_size].father_id = node_id;
		old_node_id = tree_size;
		
		//Tree is full, reset it!
		if(tree_size >= tree_max_size){
			tree_size = 256;
			clear_tree(tree, tree_max_size);
			old_node_id = -1;
		}
	}
	while(1);
	
	fflush(output);
	fclose(output);
	bitio_close(input);
	free(tree);
	free(partial_string);	
	return 1;	
}
