#include "bitio.h"
#include "hash_table.h"
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <utime.h>


/*---------SHARED RESOURCE----------*/ 
void printv(int verbosity, const char *format, ...);

inline void loadBar(int verbose, int x, int n, int r, int w);


/*---------Compress functions-------*/
int compressor(char* input_file, char* output_file, int dictionary_size, int verbose_mode);


/*---------Decompress functions------*/
int decompressor(char* input_file, int verbose_mode);
