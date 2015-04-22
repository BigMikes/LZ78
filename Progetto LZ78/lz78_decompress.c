#include "lz78.h"

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
	how_many =  (int)ceil(log((double) tree_dim) / log(2));
	//read the bits from the file 
	ret = bitio_read_chunk(fd, &node_id, how_many);
	if(ret != how_many){
		return -1;
	}
	return (int) node_id;
}

int decompressor(char* input_file, char* output_file, int verbose_mode){
	int node_id;
	int tree_size = 256;
	struct bitio* input = bitio_open(input_file, 'r');
	while((node_id = read_node(input, tree_size)) != 0){
		printf("I have read node id = %i\n", node_id);
		tree_size++;
	}
	return 1;
}
