#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
//File to key
//key_t  ftok(filepath,proj_id)
//

int main (int argc, char **argv){
key_t msgKey;
msgKey=ftok("~/bit_pi",'A'); //각 파일 패스와 key값을 매칭
printf ("ftok_key=%d\n",msgKey);

return 0;


}
