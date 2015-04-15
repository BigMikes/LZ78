compile: lz78

bitio.o: bitio.h bitio.c
	cc -c -O2 -Wall -Werror bitio.c
	
hash_table.o: hash_table.c hash_table.h
	cc -c -O2 -Wall -Werror hash_table.c

lz78_decompress.o: lz78_decompress.c bitio.h lz78.h
	cc -c -O2 -Wall -Werror lz78_decompress.c
	
lz78_compress.o: lz78_compress.c bitio.h lz78.h hash_table.h
	cc -c -O2 -Wall -Werror lz78_compress.c

lz78.o: lz78.c lz78.h lz78_decompress.c lz78_compress.c
	cc -c -O2 -Wall -Werror lz78.c
	
lz78: lz78.o bitio.o lz78_compress.o lz78_decompress.o hash_table.o
	cc -g -o lz78 bitio.o lz78.o lz78_compress.o lz78_decompress.o hash_table.o

test: hash_table.o
	cc -c -O2 -Wall -Werror testhashtable.c
	cc -g -o testhashtable hash_table.o testhashtable.o
