#include "hash_table.h"


//Single entry of the hash table
struct entry_s{
	uint32_t father_id;			//Father node identifier
	char symbol;				//The associated character to the node 
	uint32_t node_id;			//Node identifier
};


//Hash table structure
struct hashtable_s{
	int size;				//Real dimention of hash table
	int num_records;			//Number of present records in the table
	uint32_t hash_start_value;		//It will contain the Benrstein's magic number 5381 mixed with a random number
	entry_t *table;				//It will be an array of entry_t elements, the reason to keep memory to the minimum required
	int bit_len;
	int bound;
};



//Brenstein's hash function
uint32_t hash(hashtable_t* ht, unsigned char* c, int dim){
	uint32_t result = ht->hash_start_value;
	int i;
	for(i = 0; i < dim; i++)
		result = ((result << 5) + result) + c[i];		// h_i = |h_i-1 * 33 + c|
	return result;
}


/*
* Function that fills the first layer of the tree with all 255 ASCII characters
* return -1 if error occurs, 1 otherwise
*/
int fill_first_layer(hashtable_t* ht){
	char ascii;
	int i;
	int ret;
	for(i = 0; i <= 255; i++){
		ascii = (char)i;
		ret = insert(ht, ascii, 0);
		if(ret == 0){  			//Error in insert function
			return -1;
		}
	} 
	return 1;
}


//Create the hash table with a given size
hashtable_t* create_hash_table(int dict_size){
	hashtable_t *ret = NULL;
	int seed; 
	int size = 256;
	if(dict_size <= 0){
		return NULL;
	}
	
	ret = malloc(sizeof(hashtable_t));
	if(ret == NULL)
		return NULL;
		
	//Gestire fattore di espansione, cioè fare una tabella maggiore rispetto al dizionario
	size += dict_size;
	
	ret->table = calloc(size, sizeof(entry_t));
	if(ret->table == NULL){
		free(ret);
		return NULL;
	}
	
	ret->size = size;
	ret->num_records = 0;
	ret->hash_start_value = 5381;
	ret->bit_len = 8;
	ret->bound = 256;
	//Seed the random number generator (used in hash value generation)
	srand(time(NULL));			
	
	//In order to keep the hash table resistent from a possible collision attack
	seed = rand();
	ret->hash_start_value = hash(ret, (unsigned char*) &seed, sizeof(seed));
	
	//Fill the first layer of the tree
	fill_first_layer(ret);
	
	return ret;
	
}


//Cleans the hash table and re-fill the first layer of the tree
void restart_ht(hashtable_t* ht){
	
	if(ht == NULL)
		return;
	
	ht->num_records = 0;
	
	memset(ht->table, 0, sizeof(entry_t) * ht->size);
	
	ht->bit_len = 8;
	ht->bound = 256;
	
	fill_first_layer(ht);
}



//Given the key: <Father_id, Symbol> return the hash value
uint32_t key_generation(hashtable_t* ht, uint32_t father_id, char symbol){
	uint32_t hash_value;
	//The buffer that will contain the father_id concatenated with symbol 
	const int dim = sizeof(father_id) + sizeof(symbol);
	unsigned char to_hash[dim];
	
	//Concatenate the char in the most significat part of the buffer and the father_id in the least significant part
	to_hash[0] =  symbol;
	to_hash[1] =  father_id;
	to_hash[2] =  father_id >> 8;
	to_hash[3] =  father_id >> 16;
	to_hash[4] =  father_id >> 24;
	
	//Hash
	hash_value = hash(ht, to_hash, dim); 	
	
	
	return hash_value % ht->size;
}


/*
* Given the father_node_id and the symbol, search in the hashtable if that node exits
* return the identifier of node found
* the parameters called "find" will contain: -1 if an error occurs, 1 if the node is found, 0 otherwise
*/
uint32_t search(hashtable_t* ht, char symbol, uint32_t father_id, int* find){
	entry_t* node;
	uint32_t hash_val;
	int i;
	
	if(ht == NULL){
		*find = -1;
		return 0;
	}
	
	hash_val = key_generation(ht, father_id, symbol);
	
	node = &ht->table[hash_val];
	
	//Checks if it is empty 
	if(node->father_id == 0 && node->node_id == 0){
		*find = 0;
		return 0;
	}
	
	//Checks if it is a collision 
	if((node->father_id == father_id) && (node->symbol == symbol)){
		*find = 1;
		return node->node_id;
	}
	//It's a collision, thus search the entry with a linear scan
	else{
		for(i = 0; i < ht->size; i++){
			hash_val += 1;
			hash_val = hash_val % ht->size;
			node = &ht->table[hash_val];
			//If it meets an empty entry it does mean that such node can't exist at all
			//otherwise it would be in this empty entry
			if(node->father_id == 0 && node->node_id == 0){
				*find = 0;
				return 0;
			}	
			if((node->father_id == father_id) && (node->symbol == symbol)){
				*find = 1;
				return node->node_id;
			}	
		}
		//If we are at this point does mean that all the hash table was scanned and no entry was found
		*find = 0;
		return 0;
	}
	 
}  

/*
* Inserts a new node to the tree with <father_id> as father node, and <symbol> as associated symbol 
* return the identifier of node added or 0 if an error occurs
*/
uint32_t insert(hashtable_t* ht, char symbol, uint32_t father_id){
	entry_t* node;
	uint32_t hash_val;
	uint32_t ret_value; 
	
	if(ht == NULL){
		return 0;
	}
		
	hash_val = key_generation(ht, father_id, symbol);
	
	node = &ht->table[hash_val];
	
	//Checks if it is a collision
	while(node->father_id != 0 || node->node_id != 0){
		hash_val += 1;
		hash_val = hash_val % ht->size;
		node = &ht->table[hash_val];
	}
	
	node->father_id = father_id;
	node->symbol = symbol;
	ht->num_records += 1;
	node->node_id = ht->num_records;			//Increment the number of entry and use it as new node id
	ret_value = node->node_id;
	
	if(ht->num_records == ht->size)
		restart_ht(ht);
	
	return ret_value;
}


void print_ht(hashtable_t* ht){
	int i;
	entry_t* entry;
	
	if(ht == NULL)
		return;
	for(i = 0; i < ht->size; i++){
		entry = &ht->table[i];
		if(entry != NULL)
			fprintf(stderr,"Entry # = %i | Node id = %i | Symbol = %c | Father id = %i |\n", i, (int)entry->node_id, entry->symbol, (int)entry->father_id);
		else
			fprintf(stderr,"Entry # = %i | Empty\n", i);
	}	
	return;
}

/*
* Delete hash table entirely and its data structures
*/
void free_ht(hashtable_t* ht){
	if(ht == NULL)
		return;
	free(ht->table);
	free(ht);
	
}

//Returns the number of entry in the table
int get_num_records(hashtable_t* ht){
	if(ht == NULL)
		return -1;
	
	return ht->num_records;
}
//Returns the number of bits needed to represent the node_id
int get_bit_len(hashtable_t* ht){
	if(ht == NULL)
		return -1;
	//max encoding bit dimention = 32.
	if(ht->bit_len < 32){
		if(ht->num_records < ht->bound)
			return ht->bit_len;
		else{
			ht->bound *= 2;
			ht->bit_len += 1;
			return ht->bit_len;
		}
	}
	return 32;	
	
}
