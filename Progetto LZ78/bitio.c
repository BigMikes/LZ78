#include "bitio.h"
#define BITIO_BUF_WORDS 512

/*
The bitio structure works, with its functions, if and only if the machine has a little endian system and the architecture is 64 bits. 
The bits will be interpreted according to this endianess system.
*/


/*Bitio data structure*/
struct bitio{
	int bitio_fd;//File descriptor	
	char bitio_mode;//Mode: read 'r', write 'w' and so on.
	unsigned bitio_rp, bitio_wp;//The same as int, but unsigned.
	uint64_t buf[BITIO_BUF_WORDS];	//Buffer of unsigned int
};

/*------Return file descriptor------*/
int get_fd(struct bitio* bitio){
	if(bitio == NULL)
		return -1;
	return bitio->bitio_fd;
}

/*------OPEN-----*/
//Open the file that will be either read or write. It returns the data structure if all goes good, otherwise 0.
struct bitio*  bitio_open(const char* name, char mode){//I open the file which has the name is name, with a certain mode
	struct bitio* ret = NULL;//Checks about name and mode
	
	if(name == NULL || (mode != 'r' && mode != 'w')){
		errno = EINVAL;
		goto fail;
	}
	
	ret = (struct bitio*)calloc(1, sizeof(struct bitio));
	if(ret == NULL){//If ret isn't correctlty instanciated
		//errno = *A_Macro*
		goto fail;
	}
	
	ret->bitio_fd = open(name, mode == 'r' ? O_RDONLY : O_WRONLY | O_TRUNC | O_CREAT, 0666);//If mode='r' open the file in read mode. Otherwise create the file only in write mode and clear all data from the file.
	if(ret->bitio_fd < 0){
		goto fail;
	}
	//The execution gets here when I open correctly the file.
	ret->bitio_mode = mode;
	ret->bitio_rp = ret->bitio_wp = 0;
	return ret;
	
	//error handler
fail:
	if(ret != NULL)
		free(ret);
	return NULL;
}

/*----- FLUSH -----*/

int bitio_flush(struct bitio* b){//If the mode isn't write mode, error.
	int len_bytes;
	int left;
	char* dst=NULL;
	int i;
	
	if(b == NULL || b->bitio_mode != 'w' || b->bitio_fd < 0){
		errno = EINVAL;
		return -1;
	}
	//Computes the number of aligned bits (remember that you can store a char, that is 8 bits, not less)
	len_bytes = b->bitio_wp / 8;//Compute the bytes number
	if(len_bytes == 0){
		return 0;
	}
	char* start = (char*)b->buf;//The first byte to flush
	left = len_bytes;// Number of bytes to store

	//Write until there are something to write
	for(;;){
		int x;
		x = write(b->bitio_fd, start, left);	
		if(x < 0){
			goto fail;
		}
		left -= x;
		start += x;
		if(left == 0){
			break;
		}
	}
	
	if(b->bitio_wp % 8 != 0){//If there are some unaligned bits, put that at the top of the buffer
		dst = (char*)(b->buf);	//Pointer to the head of buffer
		dst[0] = start[0];//start[0] will contain the unaligned bits
	}
	b->bitio_wp = b->bitio_wp % 8;
	return len_bytes * 8;

	//error handler
fail:
	dst = (char*)(b->buf);
	for(i = 0; i < len_bytes - left; i++){			//in case of error, shift in any case the bytes that were flushed
		dst[i] = start[i];
	}
	if(b->bitio_wp % 8 != 0){						//if there are unaligned bits, shift also them
		dst[i] = start[i];
	}
	b->bitio_wp -= (len_bytes - left) * 8;			//adjust the pointer 
	return -1;
}

/*----- READ ----*/
//This function read the bytes from file and put them in the buffer.
int bitio_read(struct bitio* b){
	uint64_t d;
	int x;
	int pos, ofs;
	int dim_of_a_word;
	
	//Some checks
	dim_of_a_word = sizeof(b->buf[0]) * 8;	
	if(b == NULL || b->bitio_mode != 'r' || b->bitio_fd < 0){//Check on b, read mode and fd.
		errno = EINVAL;
		return -1;
	}
	if(b->bitio_rp == b->bitio_wp){//If the read and write pointer are in the same position they assume position 0
		b->bitio_rp = b->bitio_wp = 0;//Initally the buffer is completly empty
		for(;;){
			x = read(b->bitio_fd, b->buf, sizeof(b->buf));//I read from the file. 
			if(x < 0)
				return -1;
			else if(x == 0){
				continue;
			}
			else{
				b->bitio_wp = x * 8;
				break;
			}
		}
	}

	pos = b->bitio_rp / dim_of_a_word;
	ofs = b->bitio_rp % dim_of_a_word;
	d = le64toh(b->buf[pos]);
	d = d & ((uint64_t)1 << ofs);
	b->bitio_rp++;
	return (d != 0 ? 1 : 0);
}


int fill_buffer(struct bitio* b){
	int x = 0;
	
	if(b == NULL)
		return -1;
	if(b->bitio_rp == b->bitio_wp){
		b->bitio_rp = b->bitio_wp = 0;
		for(;;){
			x = read(b->bitio_fd, b->buf, sizeof(b->buf));
			if(x < 0)
				return -1;
			else if(x == 0){
				continue;
			}
			else{
				b->bitio_wp = x * 8;
				break;
			}
		}
	}
	return 1;
}

/*----- READ a chunk of bits----*/
int bitio_read_chunk(struct bitio* b, uint64_t* buf, int dim){
	uint64_t d, mask, temp;
	int errors;
	int pos, ofs;
	int dim_of_a_word = sizeof(b->buf[0]) * 8;
	int unaligned, num_of_bits;
			
	if(b == NULL || b->bitio_mode != 'r' || b->bitio_fd < 0 || dim > 64){
		errno = EINVAL;
		return -1;
	}
	
	errors = fill_buffer(b);
	if(errors == -1)
		return -1;
	
	pos = b->bitio_rp / dim_of_a_word;
	ofs = b->bitio_rp % dim_of_a_word;
	d = le64toh(b->buf[pos]);
	
	//calcolo il numero di bit che dovrò, eventualmente, leggere dalla locazione di memoria successiva
	unaligned = (dim + ofs <= dim_of_a_word) ? 0 : ofs + dim - dim_of_a_word;
	//calcolo il numero di bit che invece leggo dalla locazione attuale
	num_of_bits = dim - unaligned;
	//1st step
	mask = (num_of_bits == 64)?~(uint64_t)0:((uint64_t)1 << num_of_bits) - 1;
	//salvo in una variabile temporanea i bit letti, che andranno concatenati come meno significativi ai bit letti dopo
	temp = (d & (mask << ofs)) >> ofs;
	b->bitio_rp += num_of_bits;
	if(unaligned > 0){
		//2nd step
		errors = fill_buffer(b);
		if(errors == -1)
			return -1;
		//aggiorno il puntatore alla locazione di memoria
		pos = b->bitio_rp / dim_of_a_word;
		d = le64toh(b->buf[pos]);
		mask = ((uint64_t) 1 << unaligned) - 1;
		//concateno i bit letti prima con quelli che estraggo da 'd' (che sono la parte più significativa, quindi vanno shiftati a sinistra)
		*buf = temp | ((d & mask) << num_of_bits);
		b->bitio_rp += unaligned;
	}
	else
		*buf = temp;	
	return dim;
}


/*------ WRITE -----*/
//This function returns 0 if the write is done correctly returns 0, otherwise -1. 
int bitio_write(struct bitio *b, int x){
	int pos, ofs;
	uint64_t d;
	int dim_of_a_word = sizeof(b->buf[0]) * 8;
	
	//Some checks
	if(b == NULL || b->bitio_mode != 'w' || b->bitio_fd < 0){
		errno = EINVAL;
		return -1;
	}
	if(b->bitio_wp == sizeof(b->buf) * 8){
		int status = bitio_flush(b);
		if(status < 0)
			return -1;		
	}

	pos = b->bitio_wp / dim_of_a_word;
	ofs = b->bitio_wp % dim_of_a_word;
	d = htole64(b->buf[pos]);
	if(x != 0)
		d |= ((uint64_t)1 << ofs);
	else
		d &= ~((uint64_t)1 << ofs);
	b->buf[pos] = htole64(d);
	b->bitio_wp++;
	return 0;
}

/*------WRITE di chunk of bits------*/
int bitio_write_chunk(struct bitio *b, uint64_t x, int dim){
	int pos, ofs;
	uint64_t d;
	int dim_of_a_word = sizeof(b->buf[0]) * 8;
	int unaligned = 0;
	int num_of_bits = 0;
	uint64_t clear, mask, temp;
	
	if(b == NULL || b->bitio_mode != 'w' || b->bitio_fd < 0 || dim > 64){
		errno = EINVAL;
		return -1;
	}
	if(b->bitio_wp + dim >= sizeof(b->buf) * 8){
		int status = bitio_flush(b);
		if(status < 0)
			return -1;		
	}
	pos = b->bitio_wp / dim_of_a_word;
	ofs = b->bitio_wp % dim_of_a_word;
	d = htole64(b->buf[pos]);

	//mi salvo in due variabili temporanee la parte di bit da scrivere nella locazione corrente
	//e l'altra parte di bit da scrivere nella locazione 'pos+1'
	unaligned = (dim + ofs <= dim_of_a_word) ? 0 : ofs + dim - dim_of_a_word;
	num_of_bits = dim - unaligned;
	//1st step
	mask = (num_of_bits == 64)?~(uint64_t)0:((uint64_t)1 << num_of_bits) - 1;
	temp = x >> num_of_bits;
	clear = d & ~(mask << ofs);					//clear	
	d = clear | ((x & mask) << ofs);				//set
	b->buf[pos] = htole64(d);
	b->bitio_wp += num_of_bits;
	if(unaligned > 0){
		//2nd step
		//aggiorno il puntatore della locazione di memoria
		pos = b->bitio_wp / dim_of_a_word;
		mask = ((uint64_t) 1 << unaligned) - 1;
		d = temp & mask;						//set
		b->buf[pos] = htole64(d);
		b->bitio_wp += unaligned;
	}
	return dim;
}

//Function that closes the file descriptor. It returns 0 if all goes well, otherwise -1.
int bitio_close(struct bitio* b){
	//Some checks
	if(b == NULL || b->bitio_fd < 0){
		errno = EINVAL;
		return -1;
	}
	if(b->bitio_mode == 'w'){
		if(b->bitio_wp % 8 != 0){
			b->bitio_wp += 8 - (b->bitio_wp % 8);
		}
		if(bitio_flush(b) < 0 || b->bitio_wp > 0){
			goto fail;
		}
	}
	close(b->bitio_fd);
	free(b);
	return 0;
	
fail:
	//error handler
	close(b->bitio_fd);
	free(b);
	return -1;
}
