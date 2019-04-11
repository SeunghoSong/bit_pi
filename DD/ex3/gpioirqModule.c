/* 3. GPIO Driver Module
 //gpio를 제어하는 char 형 device driver를 제어 컨트롤 

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
#include <linux/uaccess.h>
#include <linux/gpio.h> 

#include <linux/interrupt.h> // IRQ를 이용해 인터럽트를 받아오함
#define GPIO_MAJOR 200
#define GPIO_MINOR 0
#define GPIO_DEVICE "gpio_swled"
#define GPIO_LED 18

#define GPIO_SW 17
//#define DEBUG
#define BLK_SIZE 100
//char 형 디바이스 드라이버 구조체 
struct cdev gpio_cdev;

static char msg[BLK_SIZE]={0};

static int switch_irq; //irq핸들러 접근용의 하도록

static int gpio_open(struct inode *, struct file *);
static int gpio_close(struct inode *, struct file *);
static ssize_t gpio_read(struct file *, char *buff, size_t, loff_t *);
static ssize_t gpio_write(struct file *, const char *, size_t, loff_t *);


static int gpio_open(struct inode *inod, struct file *fil){
   //모듈 사용 카운트 증가 
    try_module_get(THIS_MODULE);// 함수를 사용할때 사용중인 모듈갯수를 counting 한다. 
                                // 사용중인 모듈이 있을경우에는  rmmod로 제거가 안된다.
    printk(KERN_INFO "GPIO opened\n");
    return 0;
}

static int gpio_close(struct inode *inod,struct file *fil){
    //모듈 사용 카운트 감소
    module_put(THIS_MODULE);
    printk(KERN_INFO "GPIO closed\n");
    return 0;
}
//커널 ->유저
static ssize_t gpio_read(struct file *inode, char *buff, size_t len,loff_t *off){
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
    

}
//유저-> 커널
static ssize_t gpio_write(struct file *fil,const char *buff, size_t len,loff_t *off){
    static int count;
    memset(msg,0,BLK_SIZE);
    
    count=copy_from_user(msg,buff,len);
    gpio_set_value(GPIO_LED, (strcmp(msg,"0")));
    printk(KERN_INFO "GPIO write:%s\n",msg);
    return count;
}

static irqreturn_t irq_func(int irq,void *data){
    //IRQ 발생 & LED가 OFF 일때
	static int flag = 0;
    static int count;

	if (!flag)
	{
		flag = 1;
		if ((irq == switch_irq) && !gpio_get_value(GPIO_LED))
			gpio_set_value(GPIO_LED, 1);
		else //IRQ발생 & LED ON일때
			gpio_set_value(GPIO_LED, 0);

		printk(KERN_INFO " Called isr_func():%d\n", count);
		count++;
	}
	else
	{
		flag = 0;
	}
		printk(KERN_INFO "CALLED Isr_func():%d \n",count);
        count++;
		
		return IRQ_HANDLED;
}

//디바이스 드라이버 함수 호출 명시
//각 함수가 호출되면 어떤함수가 호출되는지 명시
static struct file_operations gpio_fops = {
   .owner = THIS_MODULE,
   .read = gpio_read,
   .write = gpio_write,
   .open = gpio_open,
   .release = gpio_close, };

/*module_init(exit) 함수 작성*/
static int __init Module_init(void){//insmod로 적재 될때 실행되는 함수
    
    dev_t devno; // mojor(8bit) +minor(16bit); 
	int err,count;

    printk("called hello_init()\n"); //kernel log에 출력된다.
    //gpio를 제어하는 char 형 device driver를 생성방법
    //1.MKDEV()를 이용하여 문자 디바이스 드라이버를 등록한다.
    //장치 드라이버의 영역 선언
        devno =MKDEV(GPIO_MAJOR,GPIO_MINOR);

	
	/*
	int register_chrdev_region (
	dev_t  	from,
 	unsigned  	count,
 	const char * name);
	*/
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
    // gpio 권한을 얻어온다
    err=gpio_request(GPIO_LED,"LED");
    if(err==-EBUSY){
        printk(KERN_INFO "ERROR GPIO_request LED\n");
        return -1;
    }
    gpio_direction_output(GPIO_LED,0);
    
    err=gpio_request(GPIO_SW,"SW");
    if(err==-EBUSY){
        printk(KERN_INFO "ERROR GPIO_request SW\n");
        return -1;
    }
    
    //추상적으로 적어도 칩에서 알아서 처리
    switch_irq=gpio_to_irq(GPIO_SW);
    err=request_irq(switch_irq, irq_func,IRQF_TRIGGER_RISING,"switch",NULL);
    if(err)
    {
        printk(KERN_INFO"ERROR request_irp\n");
        return -1;
    
    }

    return 0;
	}

static void __exit Module_exit(void){//rmmod로 제거 될때 실행 되는 함수
	dev_t devno=MKDEV(GPIO_MAJOR,GPIO_MINOR);
    
    gpio_direction_output(GPIO_LED,0);
	
	//request irp 에서 받아온 권한을 반납한다.
	free_irq(switch_irq, NULL);

    //gpio repuest에서 받아온 권한을 반납
    gpio_free(GPIO_LED);
    gpio_free(GPIO_SW);
    //1. 문자 디바이스의 등록을 해제한다.
    unregister_chrdev_region(devno,1);

    //2. 문자 디바이스의 구조체를 삭제한다.
    cdev_del(&gpio_cdev);
	printk("Good-bye!\n");
	}


//커널 작성시 반드시 들어가야하는 함수들 

module_init(Module_init);//init(초기화)할때 hello_init 함수 호출
module_exit(Module_exit);//exit(종료)할때

//모듈 정보
MODULE_LICENSE("GPL");//  license 작성(GPL,GPLv2)  안할시에 오류 또는 warnning
MODULE_AUTHOR("Seung ho Song"); // 제작자 이름
MODULE_DESCRIPTION("HELLO MODULE"); //용도
