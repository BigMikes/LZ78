#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

/*-------------Data Structure--------*/
struct children_s;
struct entry_s;
struct hashtable_s;

typedef struct children_s children_t;
typedef struct entry_s entry_t;
typedef struct hashtable_s hashtable_t;

/*-------------Functions-------------*/

hashtable_t* create_hash_table(int size);
uint32_t key_generation(hashtable_t* ht, int node_id, char symbol);
