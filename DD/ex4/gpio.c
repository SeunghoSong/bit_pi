#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int main (int argc, char** argv){
    char buf[BUFSIZ];
    int i=0;
    int fd;
    int count;:

    memset(buf,0,BUFSIZ);

    printf("argv[1] :%s \n",argv[1]);
    
    fd=open("/dev/gpio_swled",O_RDWR);

    if(fd<0)
    {
        printf("ERROR open()\n");
        return -1;
    }

    count=write(fd, argv[1],strlen(argv[1]));
    if(count<0)
    {
        printf("ERROR write()\n");
    
    }
    count=read(fd,buf,10);
    printf("Read data:%s\n",buf);
	sleep(5);
	close(fd);
	
    return 0;
}

