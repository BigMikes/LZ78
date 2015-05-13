#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<string.h>

#define N 1
#define SIZE_BUFF 1024
#define SIZE 32
#define MAX_LEN_NAME_FILE 256

int occupied(char *buf){
int i;
for(i=0; i<SIZE_BUFF; i++){
if(buf[i]=='\0') break;
}
return i;
}

//The real dimension of name file tipically isn't maximum. 
int real_dim_name_file(char* name_file){
int i=0;
for(i=0; i<MAX_LEN_NAME_FILE; i++){
	if(name_file[i]=='\0') break;
	}
return i;
}

void prn_wrt_buf(char *buff){
int i;
for(i=0; i<40; i++){
printf("%c",buff[i]);
}
printf("\n");
}
int main(){

//It concatenates the tab to the name of file.
char* name_file=(char*)calloc(MAX_LEN_NAME_FILE,sizeof(char));
char tab_char='\t';
name_file=strcpy(name_file,"name_file");
const int size_dic=100; 
strcat(name_file,&tab_char);

//I get the metadata from the file.
struct stat* buff=(struct stat*)calloc(N,sizeof(struct stat));
unsigned char* write_buff=(char*)calloc(SIZE_BUFF,sizeof(char));
if(buff==NULL || write_buff==NULL) exit(0);

//I get the information from the file where I read from
stat("pippoVendeDroga.txt",buff);
//The order of header file data is the following: lenght name file, name file,
unsigned char* ptr_w=memcpy(write_buff,name_file,MAX_LEN_NAME_FILE);//ptr_w points to the destination of copied memory
ptr_w+=real_dim_name_file(name_file);//Then is shifted depending on the real lenght of file name

ptr_w=memcpy(ptr_w,&(buff->st_atime),sizeof(time_t));//I copy the last access date on the buffer, then is shifted
printf("size of time_t=%d\n",sizeof(time_t));
//SO FAR OK.
ptr_w+=sizeof(time_t);//With 4 works. Underastand why
ptr_w=memcpy(ptr_w,&(buff->st_mtime),sizeof(time_t));//I copy the last modification date on the buffer, then is shifted
ptr_w+=sizeof(int);

prn_wrt_buf(write_buff);
printf("Occupied:%d\n",occupied(write_buff));
int fd=open("header.txt",O_WRONLY | O_TRUNC | O_CREAT ,0666);
if(fd<0){
printf("Errore, spacco botilia, amazo familia\n");
exit(0);
}
write(fd,write_buff,SIZE_BUFF/4);
free(buff);
free(write_buff);
free(name_file);
}
