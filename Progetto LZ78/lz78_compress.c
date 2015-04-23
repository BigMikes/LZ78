#include "lz78.h"

/*Writes the encoding of the tree node on the file
* Input param: File descriptor, dimension of the tree (needed for know how many bits it have to read), and the node id to write
* Return: -1 if an error occurs, 1 if success
*/
int emit_encode(int num_records, struct bitio* output, int father_id){
	int how_many = 32;	//-----------------------------------------------------------------------------Per ora codifico tutto su 32bit
	int ret;	
	
	if(output == NULL || num_records < 0){
		return -1;
	}
	//compute the number of bits to write with logarithm in base 2
	//how_many =  (int)ceil(log((double) num_records) / log(2));
	printf("emetto %i su %i bits\n", father_id, how_many);
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
	if(input<0){
		printf("Impossible to open input file. You are sure taht exists?\n");
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
	
	do{
		if(start){
			readed_byte=getc(input);
		}
		start = 1;
		if(readed_byte==EOF){
			//termine file
			error = emit_encode(get_num_records(hashtable), output, father_id);
			error = emit_encode(get_num_records(hashtable), output, 0);
			break;
		}
		node_id = (int)search(hashtable, (char)readed_byte, father_id, &find);
		switch(find){
			case 0:
				//emetti codifica
				//la nuova ricerca deve partire dal carattere che ha fatto fallire readed_byte
				error = emit_encode(get_num_records(hashtable), output, father_id);
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

