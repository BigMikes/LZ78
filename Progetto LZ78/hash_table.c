#include "hash_table.h"

//Define a type for the hash value. Code maintenance purpose. 
typedef uint32_t hashint;



//Single entry of the hash table
struct entry_s{
	int father_id;				//Father node identifier
	char value;					//The associated character to the node 
	int node_id;				//Node identifier
	//PUNTATORE AL PADRE?
};


//Hash table structure
struct hashtable_s{
	int size;					//Real dimention of hash table
	int num_records;			//Number of present records in the table
	hashint hash_start_value;	//It will contain the Benrstein's magic number 5381 mixed with a random number
	entry_t **table;			//It will be an array of pointers to entry_t elements, the reason to keep memory to the minimum required
};



//Brenstein's hash function
hashint hash(hashtable_t* ht, unsigned char* c, int dim){
	hashint result = ht->hash_start_value;
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
	
	ret->table = calloc(size,sizeof(entry_t));
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



//Given the key: <Father_id, Symbol> return the hash value
hashint key_generation(hashtable_t* ht, int father_id, char symbol){	
	hashint hash_value;
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
