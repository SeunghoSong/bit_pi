/* 7segment driver
 

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
#include <linux/uaccess.h>
#include <linux/gpio.h> 

#include <linux/interrupt.h> // IRQ를 이용해 인터럽트를 받아오함

#define GPIO_MAJOR 200
#define GPIO_MINOR 0
#define GPIO_DEVICE "seg_d"



// seg led num
#define SEG_A   20
#define SEG_B   21
#define SEG_C   19
#define SEG_D   13
#define SEG_E   06
#define SEG_F   16
#define SEG_G   12
#define SEG_DOT 26

#define SEG_HIGH 0
#define SEG_LOW 1
//#define DEBUG
#define BLK_SIZE 100
//char 형 디바이스 드라이버 구조체 

#define L_LOW 1
#define L_HIGH 0

int seg_led[10][8]=
{ //    a    b       c       d       e       f       g       h
    {L_HIGH, L_HIGH, L_HIGH, L_HIGH, L_HIGH, L_HIGH, L_LOW,  L_LOW},    //0
    {L_LOW, L_HIGH, L_HIGH, L_LOW,  L_LOW,  L_LOW,  L_LOW,  L_LOW},        //1
    {L_HIGH,L_HIGH,L_LOW,L_HIGH,L_HIGH,L_LOW,L_HIGH, L_LOW},      //2
    {L_HIGH,L_HIGH,L_HIGH,L_HIGH,L_LOW,L_HIGH,L_LOW,L_LOW},   //3
    {L_LOW,L_HIGH,L_HIGH,L_LOW,L_LOW,L_HIGH,L_HIGH,L_LOW},   //4
    {L_HIGH,L_LOW,L_HIGH,L_HIGH,L_LOW,L_HIGH,L_HIGH,L_LOW},   //5
    {L_HIGH,L_LOW,L_HIGH,L_HIGH,L_HIGH,L_HIGH,L_HIGH,L_LOW},   //6
    {L_HIGH,L_HIGH,L_HIGH,L_LOW,L_LOW,L_HIGH,L_LOW,L_LOW},   //7 
    {L_HIGH,L_HIGH,L_HIGH,L_HIGH,L_HIGH,L_HIGH,L_HIGH,L_LOW},   //8
    {L_HIGH,L_HIGH,L_HIGH,L_HIGH,L_LOW,L_HIGH,L_HIGH,L_LOW},   //9
};

int segs[8]={SEG_A,SEG_B,SEG_C,SEG_D,SEG_E,SEG_F,SEG_G,SEG_DOT};

struct cdev gpio_cdev;

static char msg[10];

//static int switch_irq; //irq핸들러 접근용의 하도록

static int seg_open(struct inode *, struct file *);
static int seg_close(struct inode *, struct file *);
static ssize_t seg_read(struct file *, char *buff, size_t, loff_t *);
static ssize_t seg_write(struct file *, const char *, size_t, loff_t *);


static int seg_open(struct inode *inod, struct file *fil){
   //모듈 사용 카운트 증가 
    try_module_get(THIS_MODULE);// 함수를 사용할때 사용중인 모듈갯수를 counting 한다. 
                                // 사용중인 모듈이 있을경우에는  rmmod로 제거가 안된다.
    printk(KERN_INFO "GPIO opened\n");
    return 0;
}

static int seg_close(struct inode *inod,struct file *fil){
    //모듈 사용 카운트 감소
    module_put(THIS_MODULE);
    printk(KERN_INFO "GPIO closed\n");
    return 0;
}
//커널 ->유저
static ssize_t seg_read(struct file *inode, char *buff, size_t len,loff_t *off){
    /*
    int count;
    if(gpio_get_value(GPIO_LED))
        msg[0]='1';
    else
        msg[1]='0';

    strcat(msg,"from kernel");
    //kenel 영역의 데이터를 user영역으로 복사
    count=copy_to_user(buff,msg,strlen(msg)+1);
    
    printk(KERN_INFO "GPIO read:%s",msg);

    return count;
    */
return 0;
}
//유저-> 커널
static ssize_t seg_write(struct file *fil,const char *buff, size_t len,loff_t *off){
    static int count;
    int i;
    int num;
    
    memset(msg,0,10);
    printk(KERN_INFO "memset\n");
    count=copy_from_user(msg,buff,len);
     printk(KERN_INFO "copy form\n");
    num=simple_strtol(msg,NULL,10);
     printk(KERN_INFO "simple\n");
    
    for(i=0;i<8;i++)
    gpio_set_value(segs[i],seg_led[num][i]);
    
    
    //printk(KERN_INFO "GPIO write:%s\n",msg);
    return count;
}



//디바이스 드라이버 함수 호출 명시
static struct file_operations gpio_fops = {
   .owner = THIS_MODULE,
   .read = seg_read,
   .write = seg_write,
   .open = seg_open,
   .release = seg_close, };




static int __init seg_init(void){//insmod로 적재 될때 실행되는 함수
    char temp[10]={0,};
    dev_t devno; // mojor(8bit) +minor(16bit); 
	int err,count;
    int i;

    printk("called hello_init()\n"); //kernel log에 출력된다.
    //장치 드라이버의 영역 선언
        devno =MKDEV(GPIO_MAJOR,GPIO_MINOR);
	
    register_chrdev_region(devno,1,GPIO_DEVICE);// devno의 번호와  GPIO_DEVICE 이름으로1개를 dev에 등록한다.



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
    // gpio 권한을 얻어온다
      
     
    for(i=0;i<8;i++){
        memset(temp,0,sizeof(temp));
        sprintf(temp,"%d",segs[i]);
        err=gpio_request(segs[i],temp);
    if(err==-EBUSY){
        printk(KERN_INFO "ERROR GPIO_request %d\n",segs[i]);
        return -1;
    }
    gpio_direction_output(segs[i],SEG_LOW);
    }  
    return 0;
	}

static void __exit seg_exit(void){//rmmod로 제거 될때 실행 되는 함수
	dev_t devno=MKDEV(GPIO_MAJOR,GPIO_MINOR);
    
    //gpio repuest에서 받아온 권한을 반납
    int i;
     for(i=0;i<8;i++)
     {
        gpio_direction_output(segs[i],SEG_LOW);    
        gpio_free(segs[i]);
     }
    //1. 문자 디바이스의 등록을 해제한다.
    unregister_chrdev_region(devno,1);

    //2. 문자 디바이스의 구조체를 삭제한다.
    cdev_del(&gpio_cdev);
	printk("Good-bye!\n");
	}


//커널 작성시 반드시 들어가야하는 함수들 
module_init(seg_init);//init(초기화)할때 hello_init 함수 호출
module_exit(seg_exit);//exit(종료)할때

//모듈 정보
MODULE_LICENSE("GPL");//  license 작성(GPL,GPLv2)  안할시에 오류 또는 warnning
MODULE_AUTHOR("Seung ho Song"); // 제작자 이름
MODULE_DESCRIPTION("7SEGMENT_MODULE"); //용도
