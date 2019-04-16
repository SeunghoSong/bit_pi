#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "myioctl.h"
void signal_handler(int signum){
    static int count=0 ;
puts("user signal catched");
if(signum==SIGIO)
{
    puts("SIGIO");
    printf("count:%d\n",count);
}
    count++;
}


int main (int argc, char** argv){
    char buf[BUFSIZ];
    int i=0;
    int fd;
    int count;
    unsigned long temp;
    memset(buf,0,BUFSIZ);
    signal(SIGIO,signal_handler);
    printf("argv[1] :%s \n",argv[1]);
    
    fd=open("/dev/gpioled",O_RDWR);

    if(fd<0)
    {
        printf("ERROR open()\n");
        return -1;
    }
    
    sprintf(buf,"%s:%d",argv[1],getpid());
    count=write(fd,buf,strlen(buf));
    if(count<0)
    {
        printf("ERROR write()\n");
        return -1;
    
    }
    count=read(fd,buf,10);
    printf("Read data:%s\n",buf);
    //device driver: file *,cmd, arg
    temp=ioctl(fd,CMD1,10);
    printf("ipctl CMD1 %ld",temp);
    temp=ioctl(fd,CMD2,25);
    printf("ipctl CMD2 %ld",temp);
    temp=ioctl(fd,CMD3,100);
    printf("ipctl CMD3 %ld",temp);
   
	close(fd);

	
    return 0;
}

