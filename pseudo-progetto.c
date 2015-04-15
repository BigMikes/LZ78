main{
	struct options opts;
	inizializza_opzioni();
	controlla_che_opzioni_siano_valide();
	int ret;
	
	if(compressione)
		ret = compressor(opts.input_file, opts.output_file, opts.dictionary_size, opts.verbose_mode);
	else
		ret = decompressor(opts.input_file, opts.output_file, opts.verbose_mode);		//non ha bisogno della dimensione del dizionario
																						//la legge dall'header del file compresso
	if(ret == -1){ //ERROR!
		printf("Error during blablabla");
		exit(-1);
	}
	
}

/*---------------------------------FILE COMPRESSOR---------------------------------------------------*/

void print_v(int verbose, ){
	if(verbose && text != NULL)
		printf("%s\n", text);
	return;
}

int compressor(char* input, char* output, uint32_t size, int verbose){
	FILE* fd = openfile(input, read);
	FILE* fd_compresso = bitio_open(output, write);
	controllo_errori;
	hashtable_t table = create_hash_table(size);
	inizializza_hash(table);
	uint32_t padre;
	int find;
	int nodo;
	char c;
	while(fine_compressione){
		padre = 0;
		//esplora l'albero
		while(1){
			c = getc(fd);
			nodo = search(table,c, padre, &find);
			if(find == 0){
				emetti_codifica(table, padre);
				break;
			}
			
		}
		
	}
	
	
	
}
