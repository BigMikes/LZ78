#include "lz78.h"

int main(){
	
	hashtable_t* hashtable;
	hashtable = create_hash_table(100);
	if(hashtable == NULL){
		printf("ERROR IN HASH TABLE GENERATION\n");
		return 0;
	}
	int node_id = -1;
	int find;
	insert(hashtable, 'a', 0);
	insert(hashtable, 'b', 0);
	insert(hashtable, 'c', 0);
	insert(hashtable, 'd', 0);
	
	node_id = (int)search(hashtable, 'a', 0, &find);
	if(find == 1)
		printf("The node was found, node id = %i\n", node_id); 	
	else
		printf("The node wasn't found, node id = %i\n", node_id); 
	
	node_id = (int)search(hashtable, 'c', 0, &find);
	if(find == 1)
		printf("The node was found, node id = %i\n", node_id); 	
	else
		printf("The node wasn't found, node id = %i\n", node_id);
		
	node_id = insert(hashtable, 'a', node_id);
	node_id = insert(hashtable, 's', node_id);
	
	node_id = (int)search(hashtable, 'c', 0, &find);
	if(find == 1)
		printf("The node was found, node id = %i\n", node_id); 	
	else
		printf("The node wasn't found, node id = %i\n", node_id); 
	
	node_id = (int)search(hashtable, 'a', node_id, &find);
	if(find == 1)
		printf("The node was found, node id = %i\n", node_id); 	
	else
		printf("The node wasn't found, node id = %i\n", node_id);
	
	node_id = (int)search(hashtable, 'x', node_id, &find);
	if(find == 1)
		printf("The node was found, node id = %i\n", node_id); 	
	else
		printf("The node wasn't found, node id = %i\n", node_id);
	
	return 0;
}
