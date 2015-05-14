//sei un finocchio
#include "lz78.h"

#define ONE 1
#define SIZE_BUFF 284//256 (max lenght of name_file) + 28(sum of constant dimension of other fields)
#define MAX_LEN_NAME_FILE 256

//Fuction to add the header to compressed file. It asks these parameteres: dictionary size, input and output files, file descriptor of output file and metadata
int add_header(int dictionary_size, char* in_file, char* out_file, int fd, struct stat* meta_data){
	ssize_t write_ret;
	
	unsigned char* write_buff=(unsigned char*)calloc(SIZE_BUFF,sizeof(unsigned char));//The write buffer
	if(meta_data==NULL || write_buff==NULL) 
		exit(0);

	int size_name_file=strlen(in_file);//This function doesn't count the termination character
	//header size is needed to know for the write function how many bytes to write.
	int header_size=size_name_file+sizeof(dictionary_size)+sizeof(meta_data->st_atime)+sizeof(meta_data->st_mtime)+sizeof(meta_data->st_size);

	//I get the information from the file where I read from
	stat(in_file,meta_data); //Example file: the actual file will be the file where the datas are compressed from

	//First field: lenght of name file. I put in the buffer and then I shift the pointer to the buffer.
	unsigned char* ptr_w=write_buff;
	memcpy(write_buff,&size_name_file,sizeof(size_name_file));//Variable size
	ptr_w+=sizeof(size_name_file);

	//Second field: filename. I put in the buffer and then I shift the pointer to the buffer. //Then is shifted depending on the real lenght of file name.
	ptr_w=memcpy(ptr_w,in_file,size_name_file);
	ptr_w+=size_name_file;

	//Third field: dictionary size.
	memcpy(ptr_w,&dictionary_size,sizeof(dictionary_size));//I copy the dictionary size in the buffer
	ptr_w+=sizeof(dictionary_size);

	//Fourth field: last access time.
	memcpy(ptr_w,&(meta_data->st_atime),sizeof(meta_data->st_atime));
	ptr_w+=sizeof(meta_data->st_atime);

	//Fifth field: last modification time.
	memcpy(ptr_w,&(meta_data->st_mtime),sizeof(meta_data->st_mtime));//I copy the last modification date on the buffer, then is shifted
	ptr_w+=sizeof(meta_data->st_mtime);


	//Sixth field: I copy the size file in byte on the buffer.
	memcpy(ptr_w,&(meta_data->st_size),sizeof(meta_data->st_size));

	//Only the first field has dimension can be variable, the others are always constant.

	write_ret = write(fd,write_buff,header_size);
	
	free(write_buff);
	return  write_ret;
}

int compute_bit_len(int num){
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


/*Writes the encoding of the tree node on the file
* Input param: File descriptor, dimension of the tree (needed for know how many bits it have to read), and the node id to write
* Return: -1 if an error occurs, 1 if success
*/
int emit_encode(int verbose, int num_records, struct bitio* output, int father_id){
	int how_many;	
	int ret;	
	
	if(output == NULL || num_records < 0){
		return -1;
	}
	//compute the number of bits to write with logarithm in base 2
	how_many = compute_bit_len(num_records);
	printv(verbose, "emetto %i su %i bits, tree size = %i\n", father_id, how_many, num_records);
	//read the bits from the file 
	ret = bitio_write_chunk(output, (uint64_t)father_id, how_many);
	if(ret != how_many){
		return -1;
	}
	return 1;
}


int compressor(char* input_file, char* output_file, int dictionary_size, int verbose){
	/*------------variabili---------------------------------------------------*/
	FILE* input;
	struct bitio* output;
	hashtable_t* hashtable;
	/*------------controlli---------------------------------------------------*/
	
	input = fopen(input_file, "r");
	if(input==NULL){
		printf("Impossible to open input file. You are sure that exists?\n");
		return -1;
	}
	output = bitio_open(output_file, 'w');
	if(output==NULL){
		printf("Error with output file\n");
		return -1;
	}
	hashtable = create_hash_table(dictionary_size);
	if(hashtable == NULL){
		printf("ERROR IN HASH TABLE GENERATION\n");
		return -1;
	}

	/*------------programma---------------------------------------------------*/
	int readed_byte = 0;
	int node_id, find;
	int father_id = 0;
	int start = 1;
	int error = 0;
	
	//for the load bar
	struct stat st;
	stat(input_file, &st);
	int size = st.st_size;
	int total_read_char = 0;
	
	//Before to compress, add the header
	if(add_header(dictionary_size, input_file,output_file, get_fd(output), &st)<0){
		printf("ERROR DURING WRITING HEADER FILE\n");
		return -1;
	}
	
	do{
		if(start){
			readed_byte=getc(input);
			total_read_char++;
			loadBar(!verbose, total_read_char, size, 40, 40);
		}
		start = 1;
		if(readed_byte==EOF){
			//termine file
			error = emit_encode(verbose, get_num_records(hashtable), output, father_id);
			error = emit_encode(verbose, get_num_records(hashtable), output, 0);
			break;
		}
		node_id = (int)search(hashtable, (char)readed_byte, father_id, &find);
		switch(find){
			case 0:
				//emetti codifica
				//la nuova ricerca deve partire dal carattere che ha fatto fallire readed_byte
				printv(verbose, "Char = %c  ", (char)readed_byte); 
				error = emit_encode(verbose, get_num_records(hashtable), output, father_id);
				insert(hashtable, (char)readed_byte, father_id);
				father_id = 0;
				start = 0;
				break;
			case 1:
				//rinizia col nuovo carattere
				//aggiorna il padre
				father_id = node_id;
				break;
			case -1:
				//ritorna errore hashtable non consistente
				printf("Tabella non consistente\n");
				return -1;
				break;
		}
		//checks if some error occurs during the emitting of the encoding
		if(error == -1){
			printf("Error in writing of the encoding\n");
			return -1;
		}
		
	} while(1);
	
	
	/*---------format file----------------------------------------------------*/
	
	/*---------closing--------------------------------------------------------*/
	free_ht(hashtable);
	fclose(input);
	bitio_close(output);
	return 1;
}
