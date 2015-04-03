#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <endian.h>

#ifndef _BITIO_H_12345931_
#define _BITIO_H_12345931_

struct bitio;
struct bitio* bitio_open(const char* name, char mode); 			//mode = 'r' for read and 'w' for read&write
int bitio_write_chunk(struct bitio* f, uint64_t bit, int dim);
int bitio_write(struct bitio* f, int bit);
int bitio_read_chunk(struct bitio* b, uint64_t* buf, int dim);
int bitio_read(struct bitio* f);
int bitio_flush(struct bitio* f);
int bitio_close(struct bitio* f);

#endif /*_BITIO_H_12345931_*/
