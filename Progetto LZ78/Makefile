
# Set the compiler
CC = cc
# Set the libraries to link
LFLAGS += -lcrypto
CFLAGS += -O2 -Wall -Werror
# Debug
#CFLAGS += -g

# The target
TARGET = lz78
# Object files
OBJECTS = bitio.o lz78.o lz78_compress.o lz78_decompress.o hash_table.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LFLAGS) -o $(TARGET)

bitio.o: bitio.h bitio.c
	$(CC) $(CFLAGS) -c bitio.c
	
hash_table.o: hash_table.c hash_table.h
	$(CC) $(CFLAGS) -c hash_table.c 

lz78_decompress.o: lz78_decompress.c bitio.h lz78.h
	$(CC) $(CFLAGS) -c lz78_decompress.c
	
lz78_compress.o: lz78_compress.c bitio.h lz78.h hash_table.h
	$(CC) $(CFLAGS) -c lz78_compress.c

lz78.o: lz78.c lz78.h lz78_decompress.c lz78_compress.c
	$(CC) $(CFLAGS) -c lz78.c

clean:
	rm *.o
	rm lz78
