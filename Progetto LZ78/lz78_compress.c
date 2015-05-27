#include "lz78.h"

#define ONE 1
#define SIZE_BUFF 284//256 (max lenght of name_file) + 28(sum of constant dimension of other fields)
#define MAX_LEN_NAME_FILE 256

//This function return the first position in the buffer in which is possible to write
unsigned char* add_and_shift(unsigned char* ptr_w, const void* src, size_t size_src){
	//fprintf(stderr,"Size to put into dictionary: %d\n",(int) size_src); DEBUG PRINT
	memcpy(ptr_w,src,size_src);
	ptr_w+=size_src;
	return ptr_w;
}

//Fuction to add the header to compressed file. It asks these parameteres: dictionary size, input and output files, file descriptor of output file and metadata
int add_header(int dictionary_size, char* in_file, char* out_file, int fd, struct stat* meta_data){
	int header_size;
	int lenght_file_name=strlen(in_file);
	unsigned char* ptr_w;
	ssize_t write_ret;
	
	unsigned char* write_buff=(unsigned char*)calloc(SIZE_BUFF,sizeof(unsigned char));//The write buffer
	if(meta_data==NULL || write_buff==NULL) 
		exit(0);

	//header size is needed to know for the write function how many bytes to write.
	header_size=lenght_file_name+sizeof(dictionary_size)+sizeof(meta_data->st_atime)+sizeof(meta_data->st_mtime)+sizeof(meta_data->st_size);
	//I get the information from the file where I read from
	stat(in_file,meta_data); //Example file: the actual file will be the file where the datas are compressed from

	ptr_w=write_buff;
	ptr_w=add_and_shift(ptr_w, &lenght_file_name, sizeof(lenght_file_name));        //1st field: I put the lenght of the file name. It's always an integer
	ptr_w=add_and_shift(ptr_w, in_file, strlen(in_file));     //2nd field: filename. //Then is shifted depending on the real lenght of file name.
	ptr_w=add_and_shift(ptr_w,&dictionary_size, sizeof(dictionary_size));	   //3rd field: dictionary size.
	ptr_w=add_and_shift(ptr_w,&(meta_data->st_atime),sizeof(meta_data->st_atime)); //4th field: last access time.
	ptr_w=add_and_shift(ptr_w,&(meta_data->st_mtime),sizeof(meta_data->st_mtime)); //5th field: last modfication time.
	ptr_w=add_and_shift(ptr_w,&(meta_data->st_size),sizeof(meta_data->st_size));  //6th field: file size in byte.
	write_ret = write(fd,write_buff,header_size);
	
	free(write_buff);
	return  write_ret;
}

int add_digest(char* output_file, int verbose){
	int i;
	unsigned char data[1024];
	unsigned int digest_size = 32;
	unsigned char hash[digest_size];
	int read_bytes;
	FILE* output_sha;
	EVP_MD_CTX sha_ctx;
	
	output_sha = fopen(output_file, "a+");
	if(output_sha==NULL){
		fprintf(stderr,"Impossible to reopen output file.\n");
		return -1;
	}
	
	
	EVP_MD_CTX_init(&sha_ctx);
	
	EVP_DigestInit(&sha_ctx, EVP_sha256());
	
	while( ( read_bytes = fread(data, 1, 1024, output_sha) ) != 0 ){
		EVP_DigestUpdate(&sha_ctx, data, read_bytes);
	}
	EVP_DigestFinal(&sha_ctx, hash, &digest_size);
	
	EVP_MD_CTX_cleanup(&sha_ctx);

	if(verbose){	//this step is not necessary but it's usefull if verbose isn't needed
		for(i = 0 ; i <= 32 ; i++)
			printv(verbose, "%02x", hash[i]);
		printv(verbose, "\n");
	}
	fwrite(hash, 1, 32, output_sha);
	
	fclose(output_sha);
	
	return 0;
}

/*Writes the encoding of the tree node on the file
* Input param: File descriptor, dimension of the tree (needed for know how many bits it have to read), and the node id to write
* Return: -1 if an error occurs, 1 if success
*/
int emit_encode(int verbose, int bit_len, struct bitio* output, int father_id){	
	int ret;	
	
	if(output == NULL || bit_len < 0){
		return -1;
	}
	printv(verbose, "emetto %i su %i bits\n", father_id, bit_len);
	//read the bits from the file 
	ret = bitio_write_chunk(output, (uint64_t)father_id, bit_len);
	if(ret != bit_len){
		return -1;
	}
	return 1;
}


int compressor(char* input_file, char* output_file, int dictionary_size, int verbose){
	/*------------variabili---------------------------------------------------*/
	FILE* input;
	struct bitio* output;
	hashtable_t* hashtable;
	
	int readed_byte = 0;
	int node_id, find;
	int father_id = 0;
	int start = 1;
	int error = 0;
	
	//for the load bar
	struct stat st;
	int size;
	int total_read_char = 0;
	
	/*------------controlli---------------------------------------------------*/
	
	input = fopen(input_file, "r");
	if(input==NULL){
		fprintf(stderr,"Impossible to open input file. You are sure that exists?\n");
		return -1;
	}
	output = bitio_open(output_file, 'w');
	if(output==NULL){
		fprintf(stderr,"Error with output file\n");
		return -1;
	}
	hashtable = create_hash_table(dictionary_size);
	if(hashtable == NULL){
		fprintf(stderr,"Error in table generation\n");
		return -1;
	}

	/*------------programma---------------------------------------------------*/
	//file size
	stat(input_file, &st);
	size = st.st_size;
	
	//Before to compress, add the header
	if(add_header(dictionary_size, input_file,output_file, get_fd(output), &st)<0){
		fprintf(stderr,"ERROR DURING WRITING HEADER FILE\n");
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
			error = emit_encode(verbose, get_bit_len(hashtable), output, father_id);
			error = emit_encode(verbose, get_bit_len(hashtable), output, 0);
			break;
		}
		node_id = (int)search(hashtable, (char)readed_byte, father_id, &find);
		switch(find){
			case 0:
				//emetti codifica
				//la nuova ricerca deve partire dal carattere che ha fatto fallire readed_byte
				printv(verbose, "Char = %c  ", (char)readed_byte); 
				error = emit_encode(verbose, get_bit_len(hashtable), output, father_id);
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
				fprintf(stderr,"Table not consistent\n");
				return -1;
				break;
		}
		//checks if some error occurs during the emitting of the encoding
		if(error == -1){
			fprintf(stderr,"Error in writing of the encoding\n");
			return -1;
		}
		
	} while(1);
	
	/*---------closing--------------------------------------------------------*/
	free_ht(hashtable);
	fclose(input);
	bitio_close(output);
	if(add_digest(output_file, verbose) < 0){
		return -1;
	}
	
	return 1;
}
