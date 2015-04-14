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
	entry_t **table;			//It will be an array of pointers to entry_t elements, the reason to keep memory to the minimum required
};



//Brenstein's hash function
uint32_t hash(hashtable_t* ht, unsigned char* c, int dim){
	uint32_t result = ht->hash_start_value;
	int i;
	for(i = 0; i < dim; i++)
		result = ((result << 5) + result) + c[i];		// h_i = |h_i-1 * 33 + c|
	return result;
}


//Create the hash table with a given size
hashtable_t* create_hash_table(int size){
	hashtable_t *ret = NULL;
	int seed; 
	if(size <= 0)
		return NULL;
	
	ret = malloc(sizeof(hashtable_t));
	if(ret == NULL)
		return NULL;
		
	//-----------------------------------------------------------------Gestire fattore di espansione, cioè fare una tabella maggiore rispetto al dizionario
	
	ret->table = calloc(size, sizeof(entry_t * ));
	if(ret->table == NULL){
		free(ret);
		return NULL;
	}
	
	ret->size = size;
	ret->num_records = 0;
	ret->hash_start_value = 5381;
	//Seed the random number generator (used in hash value generation)
	srand(time(NULL));			
	
	//In order to keep the hash table resistent from a possible collision attack
	seed = rand();
	ret->hash_start_value = hash(ret, (unsigned char*) &seed, sizeof(seed));
	
	return ret;
	
}


//Cleans the hash table
void clean_up(hashtable_t* ht){
	
}



//Given the key: <Father_id, Symbol> return the hash value
uint32_t key_generation(hashtable_t* ht, uint32_t father_id, char symbol){	
	uint32_t hash_value;
	//The buffer that will contain the father_id concatenated with symbol 
	unsigned char* to_hash;
	int dim = sizeof(father_id) + sizeof(symbol);
	
	to_hash = malloc(dim);
	
	*to_hash = (father_id << (sizeof(symbol) * 8)) | symbol;
	
	//Hash
	hash_value = hash(ht, to_hash, dim); 	
	
	free(to_hash);
	
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
	
	node = ht->table[hash_val];
	
	if(node == NULL){
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
			node = ht->table[hash_val];	
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
	
	if(ht == NULL){
		return 0;
	}
	
	//if the hash table is full, then call the clean_up function
	if(ht->num_records == ht->size){
		clean_up(ht);
	}
		
	node = malloc(sizeof(entry_t));
	node->father_id = father_id;
	node->symbol = symbol;
	node->node_id = ++ht->num_records;			//Increment the number of entry and use it as new node id
		
	hash_val = key_generation(ht, father_id, symbol);
	
	//Checks if it is a collision
	while(ht->table[hash_val] != NULL){
		hash_val += 1;
		hash_val = hash_val % ht->size;
	}
	ht->table[hash_val] = node;
	
	return node->node_id;
}



