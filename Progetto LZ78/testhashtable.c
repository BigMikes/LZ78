#include "lz78.h"

int main(int argc, char* argv[]){	
	hashtable_t* hashtable;
	hashtable = create_hash_table(300);
	if(hashtable == NULL){
		fprintf(stderr,"ERROR IN HASH TABLE GENERATION\n");
		return 0;
	}
	int i;
	int node_id_1 = -1;
	int node_id_2 = -1;
	node_id_1 = insert(hashtable, 'a', 0);
	node_id_2 = insert(hashtable, 'a', 0);
	printf(stderr, "Collision: node_id_1 = %i, node_id_2 = %i\n", node_id_1, node_id_2);
	insert(hashtable, 'b', 0);
	insert(hashtable, 'c', 0);
	insert(hashtable, 'd', 0);
	//Collisions testing
	for(i = 0; i < 100; i++){
		printf(stderr, "Num of entries = %i\n", get_num_records(hashtable));
		insert(hashtable, (char) i, i);
	}
	
	print_ht(hashtable);
	
	free_ht(hashtable);
	return 0;
}
