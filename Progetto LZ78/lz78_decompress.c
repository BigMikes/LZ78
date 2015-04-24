#include "lz78.h"

struct node{
	int father_id;
	char symbol;
};


/*Reads the encoding of the tree node from the file
* Input param: File descriptor, dimension of the tree (needed for know how many bits it have to read)
* Return: -1 if an error occurs, otherwise the node identifier
*/
int read_node(struct bitio* fd, int tree_dim){
	uint64_t node_id;
	int how_many = 32;	//-----------------------------------------------------------------------------Per ora codifico tutto su 32bit
	int ret;
	if(fd == NULL || tree_dim < 0){
		return -1;
	}
	//compute the number of bits to read with logarithm in base 2
	//how_many =  (int)ceil(log((double) tree_dim) / log(2));
	//read the bits from the file 
	ret = bitio_read_chunk(fd, &node_id, how_many);
	if(ret != how_many){
		return -1;
	}
	return (int) node_id;
}


void retrieve_string(struct node* tree, int node_id, char* partial_string, char* inverse_string, int *size_array){
	int i;
	int counter = 0;
	int previous_id = node_id;
	
	//reset strings
	memset(partial_string, 0, *size_array * sizeof(char));
	memset(inverse_string, 0, *size_array * sizeof(char));
	
	while(1){
		if(counter == (*size_array)-1){
			*size_array = *size_array + 1;
			partial_string = (char*)realloc(partial_string, *size_array * sizeof(char));
			inverse_string = (char*)realloc(inverse_string, *size_array * sizeof(char));
			memset(partial_string, 0, *size_array * sizeof(char));
			memset(inverse_string, 0, *size_array * sizeof(char));
		}
		inverse_string[counter] = (char)tree[previous_id].symbol - 1;
		previous_id = tree[previous_id].father_id;
		if(previous_id == 0 ){
			break;
		}
		counter++;
	}
	
	//invert string
	for(i = counter ; i >= 0 ; i--){
		partial_string[counter-i] = inverse_string[i];
	}
	printf("--- diretta: %s\n", partial_string);
	printf("--- inversa: %s\n", inverse_string);
}


int decompressor(char* input_file, char* output_file, int verbose_mode){
	struct bitio* input = bitio_open(input_file, 'r');
	if(input==NULL){
		printf("Error with input file. You are sure that exists?\n");
		return -1;
	}
	
	FILE* output = fopen(output_file, "w");	
	if(output<0){
		printf("Impossible to open output file.\n");
		return -1;
	}
	
	int i;
	int node_id;
	int tree_size = 256;	//it's the next node identifier
	int max_nodes = 300;	///////////////////////////da prelevare dall' header
	
	int size_array = 50;	//default value
	char* partial_string = (char*)malloc( size_array * sizeof(char) );
	char* inverse_string = (char*)malloc( size_array * sizeof(char) );
	
	struct node tree[255 + max_nodes];
	char ascii;
	int father_id_old;
	
	//inizializzo array
	for(i = 0; i <= 255; i++){
		ascii = (char)i;
		tree[i].father_id = 0;
		tree[i].symbol = ascii;
	} 
	
	node_id = read_node(input, tree_size);
	printf("I have read node id = %i\n", node_id);
	
	retrieve_string(tree, node_id, partial_string, inverse_string, &size_array);
	printf("stringa letta: %s\n", partial_string);
	fwrite (partial_string , sizeof(char), strlen(partial_string), output);//change strlen()
	father_id_old = node_id;
	char char_old = partial_string[0];
	
	do{
		printf("char: %c \n", char_old);
		node_id = read_node(input, tree_size);
		if(node_id==0)
			break;
		printf("I have read node id = %i\n", node_id);
		
		retrieve_string(tree, node_id, partial_string, inverse_string, &size_array);
		printf("stringa letta: %s\n", partial_string);
		fwrite (partial_string , sizeof(char), strlen(partial_string), output);//change strlen()
		tree[tree_size].symbol = char_old;
		tree[tree_size].father_id = father_id_old;
		father_id_old = node_id;
		char_old = partial_string[0];
		tree_size++;
		
		//reset condition
		if(tree_size == (255 + max_nodes)){
			tree_size = 256;
		}
	}while(1);
	
	fclose(output);
	bitio_close(input);
	return 1;
	
}
