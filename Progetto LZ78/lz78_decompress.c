#include "lz78.h"

struct node{
	int father_id;
	char symbol;
};




/*Reads the encoding of the tree node from the file
* Input param: File descriptor, dimension of the tree (needed for know how many bits it have to read)
* Return: -1 if an error occurs, otherwise the node identifier
*/
int read_node(int verbose_mode, struct bitio* fd, int tree_dim, int* bit_len, int* bound){
	uint64_t node_id;
	int how_many; 
	int ret;
	if(fd == NULL || tree_dim < 0){
		return -1;
	}
	//compute the number of bits to read with logarithm in base 2
	how_many = compute_bit_len(tree_dim, bit_len, bound);
	
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
	//memset(partial_string, 0, *size_array * sizeof(char));
	
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


/*
* Gets the metadata from the header. Opens the output file and sets its metadata.
* Returns -1 if some error occurs, or if the user decides to not decompress the file.
* Otherwise returns the dimention of the dictoriary
*/
int get_info_from_header(int input_fd, char** output_file, int* orig_file_dim, struct utimbuf* timestamps){
	unsigned char* header;
	int dim_header;
	int name_len;
	int error;
	char response;
	int dict_size;
	time_t la_time;
	time_t lm_time;
	int temp = 0;
	
	//Reads the file name dimention (it will be the only variable field)
	error = read(input_fd, &name_len, sizeof(int));
	if(error != sizeof(int))
		return -1;
	
	//Computes the dimention of entire header (two integers for dict_size and original_file_size, and two time_t)
	dim_header = name_len + 2 * sizeof(int) + 2 * sizeof(time_t);
	
	//Allocates the buffer for the header
	header = malloc(dim_header);
	if(header == NULL){
		return -1;		
	}
	
	/*DEBUG*/ printf("%s\n", header);
	
	//Reads all the header 
	error = read(input_fd, header, dim_header);
	if(error != dim_header){
		free(header);
		return -1;
	}
	
	/*---Ask to the user if he/she wants to continue---*/
	memcpy(orig_file_dim, header + (dim_header - sizeof(int)), sizeof(int)); 
	printf("Do you want decompress %i bytes (yes/no): ", *orig_file_dim); 
	error = scanf("%c", &response);			//Temporanea, serve per rimuovere dal buffer di input il '\n' inserito dall'utente nel main
	error = scanf("%c", &response);
	if(response == 'n' || error == EOF){
		printf("\nOk, I'm terminating\n");
		free(header);
		return -1;
	}
	
	/*---Parse the content of the header---*/
	
	//Parse file name
	*output_file = malloc(name_len);
	if(*output_file == NULL){
		free(header);
		return -1;
	}
	memcpy(*output_file, header, name_len);
	temp += name_len;	
	
	printf("File name = %s\n", *output_file);
		
	//Parse the dictionary size
	memcpy(&dict_size, header + temp, sizeof(int));	
	temp += sizeof(int);
	
	printf("Dict Size = %i\n", dict_size);
	
	//Parse the last access time and last modification time
	memcpy(&la_time, header + name_len + sizeof(int), sizeof(time_t));
	temp += sizeof(time_t);
	memcpy(&lm_time, header + name_len + sizeof(time_t) + sizeof(int), sizeof(time_t));
	temp += sizeof(time_t);
	
	//Store the access time and modification time
	timestamps->actime = la_time;
	timestamps->modtime = lm_time;
	
	free(header);
	return dict_size;
}

int check_digest(char* input_file, int verbose){
	//VARIABLES
	//generic
	int i, n;
	
	//hash generation
	EVP_MD_CTX sha_ctx;
	unsigned char data[1024];
	unsigned int digest_size = 32;
	unsigned char hash_new[digest_size];
	unsigned char hash_old[digest_size];
	int read_bytes, total_read_bytes;
	
	//file stream
	FILE* input_sha;
	
	//info
	struct stat st;
	int cipher_size;
	
	//open file in reading only mode
	input_sha = fopen(input_file, "r");
	if(input_sha==NULL){
		printf("Impossible to open input file.\n");
		return -1;
	}
	
	//cipher file size
	stat(input_file, &st);
	cipher_size = st.st_size;
	// the file is cipher_size long and the hash_old is at (cipher_size-32)
	
	//evaluate new hash
	EVP_MD_CTX_init(&sha_ctx);
	
	EVP_DigestInit(&sha_ctx, EVP_sha256());
	
	total_read_bytes = 0;
	n = cipher_size / sizeof(data);
	if(cipher_size % sizeof(data) < 32 && n != 0){
		n -= 1;
	}
	//read files at block until last block of at least 32 bytes
	for(i = 0 ; i < n ; i++){
		read_bytes = fread(data, 1, 1024, input_sha);
		total_read_bytes += read_bytes;
		EVP_DigestUpdate(&sha_ctx, data, read_bytes);

	}
	//read residual data
	read_bytes = fread(data, 1, cipher_size - total_read_bytes - 32, input_sha);
	EVP_DigestUpdate(&sha_ctx, data, read_bytes);
	
	EVP_DigestFinal(&sha_ctx, hash_new, &digest_size);
	
	EVP_MD_CTX_cleanup(&sha_ctx);
	
	if(verbose){
		printv(verbose, "New evaluated hash:\n");
		for(i = 0 ; i <= 32 ; i++)
			printv(verbose, "%02x", hash_new[i]);
		printv(verbose, "\n");
	}
	
	//read hash_old
	read_bytes = fread(hash_old, 1, digest_size, input_sha);	//se tutto funziona bene 32 è quello che rimane
	if(verbose){
		printv(verbose, "Old hash:\n");
		for(i = 0 ; i <= 32 ; i++)
			printv(verbose, "%02x", hash_old[i]);
		printv(verbose, "\n");
	}
	if(CRYPTO_memcmp(hash_new, hash_old, digest_size) != 0){
		printf("Corrupted file.\n\n");
		return -1;
	}
	printf("Untouched file.\n\n");
	
	fclose(input_sha);
	
	return 0;
}

int decompressor(char* input_file, int verbose_mode){
	struct bitio* input;
	char* output_file;
	FILE* output;
	int dictionary_size;
	struct node* tree;
	int tree_max_size = 256;
	int tree_size = 256;	//First id starts from 257
	int node_id;
	int old_node_id; 
	char* partial_string;
	int string_len;
	struct utimbuf* timestamps;
	int orig_file_size; 				/*---Dimention of decompressed file---*/
	int writed_byte = 0;
	int bound = 256;
	int bit_len = 8;
		
	//check digest
	if(check_digest(input_file, verbose_mode) < 0){
		return -1;
	}
	
	//Open the compressed file
	input = bitio_open(input_file, 'r');
	if(input==NULL){
		printf("Error with input file. You are sure that exists?\n");
		return -1;
	}
	
	//Collect information from the header
	timestamps = malloc(sizeof(struct utimbuf));
	dictionary_size = get_info_from_header(get_fd(input), &output_file, &orig_file_size, timestamps);
	if(dictionary_size == -1){
		bitio_close(input);
		return -1;
	}
	
	//Open the (decompressed) output file
	output = fopen(output_file, "w");	
	if(output == NULL){
		printf("Impossible to open output file.\n");
		return -1;
	}
	
	//Allocate the tree structure
	tree_max_size += dictionary_size;
	tree = (struct node*)calloc(tree_max_size, sizeof(struct node));
	partial_string = (char*)malloc( tree_max_size * sizeof(char));
	//Initialization the firt layer of the tree
	init_tree(tree);
	old_node_id = -1;
	//Decompress
	do{
		node_id = read_node(verbose_mode, input, tree_size, &bit_len, &bound);
		//If the node is 0 we are at the end of the file
		if(node_id == 0)
			break;
		//Extract the string from the tree 
		retrieve_string(verbose_mode, tree, node_id, partial_string, &tree_max_size, &string_len);
		printv(verbose_mode, "Stampo stringa = %.*s\n", string_len, partial_string + (tree_max_size - string_len));
		writed_byte += fwrite (partial_string + (tree_max_size - string_len), sizeof(char), string_len, output);
		
		//loadbar
		loadBar(!verbose_mode, writed_byte, orig_file_size, 40, 40);
		
		//Update the symbol of the old entry 
		if(old_node_id > 0){					//This check is needed for the first cycle, where there are no entry to update.
			tree[old_node_id].symbol = partial_string[tree_max_size - string_len];
			printv(verbose_mode, "Aggiorno Old Node = %i con symbol = %c, padre id = %i\n", old_node_id, tree[old_node_id].symbol, tree[old_node_id].father_id);
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
	//bitio_close(input);
	
	//Given the access time and modification time updates the metadata of the output file
	utime(output_file, timestamps);
	
	free(partial_string);
	free(tree);
	free(output_file);
	free(timestamps);	
	return 1;	
}
