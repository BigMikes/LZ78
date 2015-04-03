compile: lz78

bitio.o: bitio.h bitio.c
	cc -c -O2 -Wall -Werror bitio.c

lz78_decompress.o: lz78_decompress.c bitio.h lz78.h
	cc -c -O2 -Wall -Werror lz78_decompress.c
	
lz78_compress.o: lz78_compress.c bitio.h lz78.h
	cc -c -O2 -Wall -Werror lz78_compress.c

lz78.o: lz78.c lz78.h lz78_decompress.c lz78_compress.c
	cc -c -O2 -Wall -Werror lz78.c
	
lz78: lz78.o bitio.o lz78_compress.o lz78_decompress.o
	cc -o lz78 bitio.o lz78.o lz78_compress.o lz78_decompress.o

