/* 2. GPIO Driver Module
 //gpio를 제어하는 char 형 device driver를 생성

 !!!!!!! 작성시 주의 !!!!!! */
/*
** 쉘 명령어**
insmod : 모듈을 커널에 적재
rmmod  : 모듈을 커널에서 제거
lsmod  : kernel에 적재된 모듈들을 볼수 있다.
dmesg  : 명령어로는 kernel에 대한 log 기록을 볼수 있다.
 */


//커널 작성시 필요한 헤더파일
#include <linux/init.h> // 초기화시 필요
#include <linux/module.h>//커널 모듈 필요
#include <linux/kernel.h>// 커널 시스템 사용에 ㅎ필요

#include <linux/fs.h>   //파일 시스템 사용
#include <linux/cdev.h> //char형 
#include <linux/io.h> //커널에서 사용하는 입출력
#include <asm/uaccess.h>

#define GPIO_MAJOR 200
#define GPIO_MINOR 0
#define GPIO_DEVICE "gpioled"
#define GPIO_LED 18
#define GPIO_SIZE 0xB4

//Raspberry pi3 PHYSICAL I/O Peripherals BaseAddr //start offset
#define BCM_IO_BASE 0x3F000000

//GPIO BASE Addr
#define GPIO_BASE (BCM_IO_BASE + 0x200000)    //GPESEL 0  offset 0x200000

//GPIO Function Select Register 0 ~ 5
#define GPIO_IN(g) (*(gpio+((g)/10))&=~(7<<(((g)%10)*3))) //해당 레지스터번지 &=~사용할 포트 인풋 설정 값000
//해당 레지스터번지 |=사용할 포트 아웃풋 설정 값001
#define GPIO_OUT(g) {(*(gpio+((g)/10))&=~(7<<(((g)%10)*3)));
                    (*(gpio+((g)/10))|=(1<<(((g)%10)*3)));}                                                    

//GPIO Function set Register 0 /clear 0 Set 1
					//  addr    value
#define GPIO_SET(g) (*(gpio+7)=(1<<g))
#define GPIO_CLR(g) (*(gpio+10)=(1<<g))

//GPIO status reg
#define GPIO_GET(g) (*(gpio+13)&1<<g)

#define DEBUG
//char 형 디바이스 드라이버 구조체 
struct cdev gpio_cdev;

volatile unsigned int *gpio;


//디바이스 드라이버 함수 호출 명시
//각 함수가 호출되면 어떤함수가 호출되는지 명시
static struct file_operations gpio_fops={
    .owner  =THIS_MODULE, 
//    .read   =gpio_read,
//    .write  =gpio_write,
//    .open   =gpio_open,
//    .release    =gpio_close,
};


/*module_init(exit) 함수 작성*/
static int __init Module_init(void){//insmod로 적재 될때 실행되는 함수
    
    dev_t devno; // mojor(8bit) +minor(16bit); 
	int err,count;

    static void* map;
    printk("called hello_init()\n"); //kernel log에 출력된다.
    //gpio를 제어하는 char 형 device driver를 생성방법
    //1.MKDEV()를 이용하여 문자 디바이스 드라이버를 등록한다.
    //장치 드라이버의 영역 선언
    devno =MKDEV(GPIO_MAJOR,GPIO_MINOR);
    
    register_chrdev_region(devno,1,GPIO_DEVICE);// devno의 번호와  GPIO_DEVICE 이름으로1개를 dev에 등록한다.

#ifdef DEBUG
    //devno 확인
    printk(KERN_INFO "devno= %d\n",devno);
#endif

    //2. 문자 디바이스를 위한 구조체를 초기화 한다.
    cdev_init(&gpio_cdev,&gpio_fops);
    gpio_cdev.owner = THIS_MODULE;
    count=1;
   
    
    //3. 문자 디바이스를 추가
    err=cdev_add(&gpio_cdev,devno,count);
    
    if(err<0){
        printk(KERN_INFO "ERROR :cdev_add()\n");
    }
    // char 디바이스 드라이버의 노드를 생성 ->접근 권한 변경 ->dev에서 드라이버를 접근할수 있도록함 
    printk(KERN_INFO "'sudo mknod /dev/%s c %d 0'\n",GPIO_DEVICE, GPIO_MAJOR);
    printk(KERN_INFO "'sudo chmod 666 /dev/%s'\n",GPIO_DEVICE);
    
    //4. 물리메모리 번지로 인자값을 전달하면 가상메모리 번지를 리턴한다. 레지스트리 주소 값을 가르키기위함
    map = ioremap(GPIO_BASE, GPIO_SIZE);

    
    if(!map){
        printk(KERN_INFO "ERROR : mapping GPIO memory\n");
        iounmap(map);
        return -EBUSY;
    }
    
    gpio=(volatile unsigned int*) map;
    //GPIO 설정
    GPIO_OUT(GPIO_LED);
    GPIO_SET(GPIO_LED);

    return 0;
	}

static void __exit Module_exit(void){//rmmod로 제거 될때 실행 되는 함수
	dev_t devno=MKDEV(GPIO_MAJOR,GPIO_MINOR);
    
    GPIO_CLR(GPIO_LED);
    //1. 문자 디바이스의 등록을 해제한다.
    unregister_chrdev_region(devno,1);

    //2. 문자 디바이스의 구조체를 삭제한다.
    cdev_del(&gpio_cdev);
// gpio 주소 반납
    if(gpio)
            iounmap(gpio);
    printk("called hello_exit()\n");
	}

//커널 작성시 반드시 들어가야하는 함수들 

module_init(Module_init);//init(초기화)할때 hello_init 함수 호출
module_exit(Module_exit);//exit(종료)할때

//모듈 정보
MODULE_LICENSE("GPL");//  license 작성(GPL,GPLv2)  안할시에 오류 또는 warnning
MODULE_AUTHOR("Seung ho Song"); // 제작자 이름
MODULE_DESCRIPTION("HELLO MODULE"); //용도
