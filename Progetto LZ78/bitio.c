#include "bitio.h"
#define BITIO_BUF_WORDS 512

struct bitio{
	int bitio_fd;
	char bitio_mode;
	unsigned bitio_rp, bitio_wp;
	uint64_t buf[BITIO_BUF_WORDS];	
};

/*------OPEN-----*/
struct bitio*  bitio_open(const char* name, char mode){
	struct bitio* ret = NULL;
	if(name == NULL || (mode != 'r' && mode != 'w')){
		errno = EINVAL;
		goto fail;
	}
	
	ret = (struct bitio*)calloc(1, sizeof(struct bitio));
	if(ret == NULL){
		//errno = *A_Macro*
		goto fail;
	}
	
	ret->bitio_fd = open(name, mode == 'r' ? O_RDONLY : O_WRONLY | O_TRUNC | O_CREAT, 0666);
	if(ret->bitio_fd < 0){
		goto fail;
	}
	
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
int bitio_flush(struct bitio* b){
	if(b == NULL || b->bitio_mode != 'w' || b->bitio_fd < 0){
		errno = EINVAL;
		return -1;
	}
	//computes the number of aligned bits (remember that you can store a char, that is 8 bits, not less)
	int len_bytes = b->bitio_wp / 8;
	if(len_bytes == 0){
		return 0;
	}
	char* start = (char*)b->buf;					//the first byte to flush
	int left = len_bytes;							//number of bytes to store
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
	char* dst;
	if(b->bitio_wp % 8 != 0){						//if there are some unaligned bits, put that at the top of the buffer
		dst = (char*)(b->buf);						//pointer to the head of buffer
		dst[0] = start[0];						//start[0] will contain the unaligned bits
	}
	b->bitio_wp = b->bitio_wp % 8;
	return len_bytes * 8;

	//error handler
fail:
	dst = (char*)(b->buf);
	int i;
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
int bitio_read(struct bitio* b){
	uint64_t d;
	int x;
	int pos, ofs;
	int dim_of_a_word = sizeof(b->buf[0]) * 8;	
	if(b == NULL || b->bitio_mode != 'r' || b->bitio_fd < 0){
		errno = EINVAL;
		return -1;
	}
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
	pos = b->bitio_rp / dim_of_a_word;
	ofs = b->bitio_rp % dim_of_a_word;
	d = le64toh(b->buf[pos]);
	d = d & ((uint64_t)1 << ofs);
	b->bitio_rp++;
	return (d != 0 ? 1 : 0);
}


/*----- READ a chunk of bits----*/
int bitio_read_chunk(struct bitio* b, uint64_t* buf, int dim){
	uint64_t d, mask, temp;
	int x;
	int pos, ofs;
	int dim_of_a_word = sizeof(b->buf[0]) * 8;
	int unaligned, num_of_bits;
			
	if(b == NULL || b->bitio_mode != 'r' || b->bitio_fd < 0 || dim > 64){
		errno = EINVAL;
		return -1;
	}
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
	
	pos = b->bitio_rp / dim_of_a_word;
	ofs = b->bitio_rp % dim_of_a_word;
	d = le64toh(b->buf[pos]);
	//controllo se i bit da leggere sono tutti nella locazione di memoria puntata o se una parte è nella locazione successiva
	if(dim + ofs <= dim_of_a_word){
		mask = (dim == 64)?~(uint64_t)0:((uint64_t)1 << dim) - 1;
		//shiftando la maschera di bit ad 1 estraggo da 'd' i bit che mi servono, li ri-shifto verso destra per mettere il bit meno significativo
		//nella posizione meno significativa del buffer da passare al chiamante  
		*buf = (d & (mask << ofs)) >> ofs;
		b->bitio_rp += dim;
	}
	else{
		//calcolo il numero di bit che dovrò leggere dalla locazione di memoria successiva
		unaligned = ofs + dim - dim_of_a_word;
		//calcolo il numero di bit che invece leggo dalla locazione attuale
		num_of_bits = dim - unaligned;
		//1st step
		mask = ((uint64_t) 1 << num_of_bits) - 1;
		//salvo in una variabile temporanea i bit letti, che andranno concatenati come meno significativi ai bit letti dopo
		temp = (d & (mask << ofs)) >> ofs;
		b->bitio_rp += num_of_bits;
		//2nd step
		//aggiorno il puntatore alla locazione di memoria
		pos = b->bitio_rp / dim_of_a_word;
		d = le64toh(b->buf[pos]);
		mask = ((uint64_t) 1 << unaligned) - 1;
		//concateno i bit letti prima con quelli che estraggo da 'd' (che sono la parte più significativa, quindi vanno shiftati a sinistra)
		*buf = temp | ((d & mask) << num_of_bits);
		b->bitio_rp += unaligned;
	}
	return dim;
}


/*------ WRITE -----*/
int bitio_write(struct bitio *b, int x){
	int pos, ofs;
	uint64_t d;
	int dim_of_a_word = sizeof(b->buf[0]) * 8;
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
	if(b->bitio_wp == sizeof(b->buf) * 8){
		int status = bitio_flush(b);
		if(status < 0)
			return -1;		
	}
	pos = b->bitio_wp / dim_of_a_word;
	ofs = b->bitio_wp % dim_of_a_word;
	d = htole64(b->buf[pos]);
	//controllo che il numero di bit che voglio scrivere ci stia all'interno della locazione di memoria
	if(ofs + dim <= dim_of_a_word){
		//se il numero di bit da scrivere sono esattamente 64, cioè il massimo, allora devo avere una maschera con tutti i bit a 1, cioè il complemento di 0
		mask = (dim == 64)?~(uint64_t)0:((uint64_t)1 << dim) - 1;
		//scrivo tutto nella locazione di memoria indicata da pos
		clear = d & ~(mask << ofs);					//clear	
		d = clear | ((x & mask) << ofs);				//se
		b->buf[pos] = htole64(d);
		b->bitio_wp += dim;
	}
	else{
		//mi salvo in due variabili temporanee la parte di bit da scrivere nella locazione corrente
		//e l'altra parte di bit da scrivere nella locazione 'pos+1'
		unaligned = ofs + dim - dim_of_a_word;
		num_of_bits = dim - unaligned;
		//1st step
		mask = ((uint64_t) 1 << num_of_bits) - 1;
		temp = x >> num_of_bits;
		clear = d & ~(mask << ofs);					//clear	
		d = clear | ((x & mask) << ofs);				//set
		b->buf[pos] = htole64(d);
		b->bitio_wp += num_of_bits;
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


int bitio_close(struct bitio* b){
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
