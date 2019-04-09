#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>

// mmap() 함수 사용을 위함
#include<sys/mman.h>

//Raspberry pi3 PHYSICAL I/O Peripherals BaseAddr //start offset
#define BCM_IO_BASE 0x3F000000

//GPIO BASE Addr
#define GPIO_BASE (BCM_IO_BASE + 0x200000)    //GPESEL 0  offset 0x200000

//GPIO Function Select Register 0 ~ 5
#define GPIO_IN(g) (*(gpio+((g)/10))&=~(7<<(((g)%10)*3))) //해당 레지스터번지 &=~사용할 포트 인풋 설정 값000
//해당 레지스터번지 |=사용할 포트 아웃풋 설정 값001
#define GPIO_OUT(g) {(*(gpio+((g)/10))&=~(7<<(((g)%10)*3)));\
                    (*(gpio+((g)/10))|=(1<<(((g)%10)*3)));}                                                    

//GPIO Function set Register 0 /clear 0 Set 1
                    //  addr    value
#define GPIO_SET(g) (*(gpio+7)=(1<<g))
#define GPIO_CLR(g) (*(gpio+10)=(1<<g))

//GPIO status reg
#define GPIO_GET(g) (*(gpio+13)&1<<g)

#define GPIO_SIZE 0xB4


volatile unsigned int *gpio; // 주소를 한칸 이동 할때마다 4 bit 씩(int 타입이기때문) 이동


int main (int argc, char **argv){
    //GPIO pin num

    int gno;
    int i,mem_fd;

    void *gpio_map;

    //핀번호가 입력이 안됐을때

    if(argc<2)
    {
        printf("Usage:%s GPIO_NO\n",argv[0]);
        return -1;
    }
    gno=atoi(argv[1]); //pin 번호 저장

    //device open /dev/mem
    if((mem_fd= open("/dev/mem",O_RDWR|O_SYNC))<0) //O_SYNC APP 단과 H/W단 까지 의 동작이 완료 되었을때까지 대기
    {
        puts("open() dev/mem error");
        return -1;
    }
/*
  void* mmap(시작주소=실제 메모리 주소 (NULL 을 집어넣을 경우 커널에서 자동으로 할당),
        사이즈,
        prot_mode 설정,
        Flags,
        file디스크립터, 메모리와fd(파일 또는 장치)를 대응 메모리에 접근할때마다 fd를 사용
        offset=가상주소
    )*/
    gpio_map=mmap(NULL,GPIO_SIZE,PROT_READ |PROT_WRITE,MAP_SHARED,mem_fd,GPIO_BASE);

    if(gpio_map==MAP_FAILED)
    {
        printf("mmap error :%d \n",(int)gpio_map);
        return -1;
    }
    
    gpio=(volatile unsigned int *)gpio_map;
    
    GPIO_OUT(gno);

    for(i=0;i<5;i++){
        GPIO_SET(gno);
        sleep(1);
        GPIO_CLR(gno);
        sleep(1);
    }
    //할당됨 메모리 영역 해제
    /*
     int munmap(시작 주소=가상주소 ,
                사이즈           
                )
     */
    munmap(gpio_map,GPIO_SIZE);
    
    return 0;
}
