#include "hash_table.h"

//Element that represents a child in the tree
struct children_s{
	int node_id;
	children_t *next;	
};


//Single entry of the hash table
struct entry_s{
	char *key;				//The key of this record, is the hash value computed from 'node_id' and 'value'
	int node_id;				//Node identifier
	char value;				//The associated character to the node 
	children_t *children;			//Children list of this node
	//PUNTATORE AL PADRE?
};


//Hash table structure
struct hashtable_s{
	int size;				//Real dimention of hash table
	int num_records;			//Number of present records in the table
	entry_t **table;			//It will be an array of pointers to entry_t elements, the reason to keep memory to the minimum required
};

//Create the hash table with a given size
hashtable_t* create_hash_table(int size){
	hashtable_t *ret = NULL;
	
	if(size <= 0)
		return NULL;
	
	ret = malloc(sizeof(hashtable_t));
	if(ret == NULL)
		return NULL;
		
	//-----------------------------------------------------------------Gestire fattore di espansione, cioè fare una tabella maggiore rispetto al dizionario
	
	ret->table = calloc(size,sizeof(entry_t));
	if(ret->table == NULL){
		free(ret);
		return NULL;
	}
	
	ret->size = size;
	ret->num_records = 0;
	
	srand(time(NULL));			//Seed the random number generator (used in hash value generation)
	
	return ret;
	
}

//Brenstein's hash function
uint32_t hash(uint32_t hash_value, char c){
	uint32_t result;
	result = ((hash_value << 5) + hash_value) + c;
	return result;
}


//Given the key <Node_id, Symbol> return the hash value
uint32_t key_generation(hashtable_t* ht, int node_id, char symbol){
	uint32_t hash_value = 5381;
	uint32_t hash_seed = rand();
	int i;
	char app;
	
	//generate the h_1 value
	for(i = 0; i < sizeof(hash_seed); i++){
		app = (hash_seed >> (i * 8)) & 255;
		hash_value = hash(hash_value, app);
	}
	
	//hash node_id
	for(i = 0; i < sizeof(node_id); i++){
		app = (node_id >> (i * 8)) & 255;
		hash_value = hash(hash_value, app);
	}
	
	//hash symbol
	hash_value = hash(hash_value, symbol);
	
	return hash_value % ht->size;
}




