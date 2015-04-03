#include "lz78.h"

int main(int argc, char* argv[]){
/*--------------Variables---------------*/
	char opt;
	const char* opt_string = "cdi:o:s:v";
	int num_opt_min = 4;
	int num_opt_max = 5;
	char* options_msg = "Use \"lz78 ['-c' compress | '-d' decompress] ['-i' <input_file>] ['-o' <output_file>] ['-v' verbose] ['-s' <dictionary_size>]\"\n";		
/*--------------Options Handle----------*/
	if(argc < num_opt_min || argc > num_opt_max){
		printf("%s", options_msg);
		exit(-1);
	}
	while((opt = getopt(argc, argv, opt_string)) != -1){
		switch(opt){
			// Option '-c' for compressor mode
			case 'c':
			
				break;
			// Option '-d' for decompressor mode
			case 'd':
			
				break;
			// Option -i <filename> for set the input file name
			case 'i':
			
				break;
			// Option -o <filename> for set the output file name
			case 'o':
			
				break;
			// Option -s <dictionary_size> for set the dictionary size
			case 's':
			
				break;
			// Option -v for verbose mode
			case 'v':
			
				break;
			default:
				printf("%s", options_msg);
				exit(-1);
		}	
	}
	return 0;
}
