#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <utime.h>
#include <openssl/evp.h>


int main(){
	//int i;
	FILE* output_sha = fopen("divina_commedia.txt", "a+");
	if(output_sha==NULL){
		printf("Impossible to open input file.\n");
		return -1;
	}
	
	unsigned char data[1024];
	unsigned int digest_size = 32;
	unsigned char hash[digest_size];
	int read_bytes;
	
	EVP_MD_CTX sha_ctx;
	EVP_MD_CTX_init(&sha_ctx);
	
	EVP_DigestInit(&sha_ctx, EVP_sha256());
	
	while( ( read_bytes = fread(data, 1, 1024, output_sha) ) != 0 ){
		EVP_DigestUpdate(&sha_ctx, data, read_bytes);
	}
	EVP_DigestFinal(&sha_ctx, hash, &digest_size);
	
	EVP_MD_CTX_cleanup(&sha_ctx);

	/*for(i = 0 ; i <= 32 ; i++)
		printf("%02x", hash[i]);
	printf("\n");*/
	
	fwrite(hash, 1, 32, output_sha);
	
	return 0;
}



/* ORIGINALE

	

*/
