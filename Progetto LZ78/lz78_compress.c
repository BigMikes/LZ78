#include "lz78.h"


void emit_encode(int num_records, struct bitio* output, int father_id){
	printf("emetto %i\n", father_id);
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
	
	do{
		if(start){
			readed_byte=getc(input);
		}
		start = 1;
		if(readed_byte==EOF){
			//termine file
			emit_encode(get_num_records(hashtable), output, father_id);
			break;
		}
		node_id = (int)search(hashtable, (char)readed_byte, father_id, &find);
		switch(find){
			case 0:
				//emetti codifica
				//la nuova ricerca deve partire dal carattere che ha fatto fallire readed_byte
				emit_encode(get_num_records(hashtable), output, father_id);
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
				printf("Tabella non consistente");
				return -1;
				break;
		}
	} while(1);
	
	
	/*---------format file----------------------------------------------------*/
	
	/*---------closing--------------------------------------------------------*/
	fclose(input);
	bitio_close(output);
	return 1;
}

