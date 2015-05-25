
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

# Dependencies:

bitio.o: bitio.h

hash_table.o: hash_table.h 

lz78_decompress.o: bitio.h lz78.h

lz78_compress.o: bitio.h lz78.h hash_table.h

lz78.o: lz78.h 

# This is a predefined rule of makefile, it knows the dependencies and thus can compile the related ".c" files in to ".o"
# The macro $< means "the first suffix of current target", i.e. ".c"
.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm *.o
	rm lz78
