#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>



int main (int argc, char** argv){
    char buf[BUFSIZ];
    int i=0;
    int fd;
    int count;

    memset(buf,0,BUFSIZ);

    printf("argv[1] :%s \n",argv[1]);
    
    fd=open("/dev/seg_d",O_RDWR);

    if(fd<0)
    {
        printf("ERROR open()\n");
        return -1;
    }

    
    if(count<0)
    {
        printf("ERROR write()\n");
    
    }
    while(1)
    
	close(fd);
	
    return 0;
}

