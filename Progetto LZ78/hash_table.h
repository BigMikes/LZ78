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
uint32_t search(hashtable_t* ht, char symbol, uint32_t father_id, int* find);
uint32_t insert(hashtable_t* ht, char symbol, uint32_t father_id);
void print_ht(hashtable_t* ht); 				//For debug purpose
void free_ht(hashtable_t* ht);
int get_num_records(hashtable_t* ht);
int get_bit_len(hashtable_t* ht);
