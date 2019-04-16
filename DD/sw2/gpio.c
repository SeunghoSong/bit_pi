#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "myswitch.h"

void signal_handler(int signum){
    static int count=0 ;
puts("user signal catched");
if(signum==SIGUSR1) 
    puts("SIGUSR1");
else if(signum==SIGUSR2)
    puts("SIGUSR2");
}


int main (int argc, char** argv){
    char buf[BUFSIZ];
    int i=0;
    int fd;
    int count;

    memset(buf,0,BUFSIZ);
    signal(SIGUSR1,signal_handler);
    signal(SIGUSR2,signal_handler);

    sprintf(buf,"/dev/%s", GPIO_DEVICE);
    fd=open(buf,O_RDWR);

    if(fd<0)
    {
        printf("ERROR open()\n");
        printf("'sudo chmod 666 /dev/%s'\n",GPIO_DEVICE);
        return -1;
    }
    
    sprintf(buf,"%d",getpid());
    count=write(fd,buf,strlen(buf));
    if(count<0)
    {
        printf("ERROR write()\n");
        return -1;
    
    }
    while(1);
	close(fd);

	
    return 0;
}

