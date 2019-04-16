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
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/timer.h>	//init_timer(), add_timer(), del_timer()
#include <linux/signal.h>	//signal을 사용
#include <linux/sched/signal.h>	//siginfo 구조체를 사용하기 위해


#define GPIO_MAJOR 200
#define GPIO_MINOR 0
#define GPIO_DEVICE "seg_d"

#define SW_STOP  23
#define SW_PAUSE 24

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

#define LOW 1
#define HIGH 0
#define MS_TO_NS(x) (x * 1E6L)


int seg_led[10][8]=
{ //    a    b       c       d       e       f       g       h
    {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, LOW,  LOW},    //0
    {LOW, HIGH, HIGH, LOW,  LOW,  LOW,  LOW,  LOW},        //1
    {HIGH,HIGH,LOW,HIGH,HIGH,LOW,HIGH, LOW},      //2
    {HIGH,HIGH,HIGH,HIGH,LOW,HIGH,LOW,LOW},   //3
    {LOW,HIGH,HIGH,LOW,LOW,HIGH,HIGH,LOW},   //4
    {HIGH,LOW,HIGH,HIGH,LOW,HIGH,HIGH,LOW},   //5
    {HIGH,LOW,HIGH,HIGH,HIGH,HIGH,HIGH,LOW},   //6
    {HIGH,HIGH,HIGH,LOW,LOW,HIGH,LOW,LOW},   //7 
    {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,LOW},   //8
    {HIGH,HIGH,HIGH,HIGH,LOW,HIGH,HIGH,LOW},   //9
};

//timer
int segs[8]={SEG_A,SEG_B,SEG_C,SEG_D,SEG_E,SEG_F,SEG_G,SEG_DOT};

struct cdev gpio_cdev; // 문자형 디바이스 드라이버 구조체
static struct hrtimer hr_timer; 	//타이머처리를 위한 구조체


//static struct task_struct *task; 	//태스크를 위한 구조체
//static int switch_irq[2]; //irq핸들러 접근용의 하도록

//static char msg[10];

pid_t pid;
char pid_valid;


static int seg_open(struct inode *, struct file *);
static int seg_close(struct inode *, struct file *);
static ssize_t seg_read(struct file *, char *buff, size_t, loff_t *);
static ssize_t seg_write(struct file *, const char *, size_t, loff_t *);

/*
static irqreturn_t isr_stop(int irq, void *data){
	
    //IRQ발생 & LED가 OFF일때 
	static int count;
	static int flag=0;
	static struct siginfo sinfo;

	if(!flag)
	{
		flag=1;
		if((irq==switch_irq) && !gpio_get_value(GPIO_LED))
		{
			gpio_set_value(GPIO_LED, 1);

			//스위치가 눌렸을 때 응용프로그램에게 SIGIO를 전달한다.
			//sinfo구조체를 0으로 초기화한다.
			memset(&sinfo, 0, sizeof(struct siginfo));		
			sinfo.si_signo = SIGIO;
			sinfo.si_code  = SI_USER;
			if(task!=NULL)
			{
				//kill()와 동일한 kernel함수
				send_sig_info(SIGIO,&sinfo,task);
			}
			else
			{
				printk(KERN_INFO "Error: USER PID\n");
			}

		}
		else //IRQ발생 & LED ON일때
			gpio_set_value(GPIO_LED, 0);
	
		printk(KERN_INFO "STOPWATCH STOP\n");
		count++;
	}
	else
	{
		flag=0;
	}
	return IRQ_HANDLED;
    
} 

static irqreturn_t isr_pause(int irq, void *data){
	
    //IRQ발생 & LED가 OFF일때 
	static int count;
	static int flag=0;
	static struct siginfo sinfo;

	if(!flag)
	{
		flag=1;
		if((irq==switch_irq) && !gpio_get_value(GPIO_LED))
		{
			gpio_set_value(GPIO_LED, 1);

			//스위치가 눌렸을 때 응용프로그램에게 SIGIO를 전달한다.
			//sinfo구조체를 0으로 초기화한다.
			memset(&sinfo, 0, sizeof(struct siginfo));		
			sinfo.si_signo = SIGIO;
			sinfo.si_code  = SI_USER;
			if(task!=NULL)
			{
				//kill()와 동일한 kernel함수
				send_sig_info(SIGIO,&sinfo,task);
			}
			else
			{
				printk(KERN_INFO "Error: USER PID\n");
			}

		}
		else //IRQ발생 & LED ON일때
			gpio_set_value(GPIO_LED, 0);
	
		printk(KERN_INFO "STOPWATCH PAUSE\n");
		count++;
	}
	else
	{
		flag=0;
	}
	return IRQ_HANDLED;
    
} //수정 필요
*/

enum hrtimer_restart timer_func(struct hrtimer *timer){
    unsigned long delay_in_ms = 100L;
    static unsigned long count =0;
    static unsigned int sec =0;
    int i;
    for(i=0;i<8;i++)
        gpio_set_value(segs[i],seg_led[sec/10][i]);   

        if(count<2) 
        {
           
	        ktime_t currtime, interval;
	        currtime = ktime_get();
	        interval = ktime_set(0, MS_TO_NS(delay_in_ms));
	        hrtimer_forward(timer, currtime, interval);
	        //pr_info("my_hrtimer_callback called (%ld).\n", jiffies);
        }
    count++;
    sec++;
    if (sec==100){
        sec=0;
        pr_info("count=%ld\n",count);
    }
    
    return HRTIMER_RESTART;
}//수정필요

static int seg_open(struct inode *inod, struct file *fil){
   //모듈 사용 카운트 증가 
   
    try_module_get(THIS_MODULE);// 함수를 사용할때 사용중인 모듈갯수를 counting 한다. 
                            // 사용중인 모듈이 있을경우에는  rmmod로 제거가 안된다.
    pr_info("GPIO opened\n");
    return 0;
}

static int seg_close (struct inode *inod,struct file *fil){
    //모듈 사용 카운트 감소
    module_put(THIS_MODULE);
    pr_info("GPIO closed\n");
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
    printk(KERN_INFO "start\n");
    return 0;
}



//디바이스 드라이버 함수 호출 명시
static struct file_operations gpio_fops = {
   .owner = THIS_MODULE,
   .read = seg_read,
   .write = seg_write,
   .open = seg_open,
   .release = seg_close, 
};




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
    
    if(err<0)
    {
        printk(KERN_INFO "ERROR :cdev_add()\n");
    }

    // char 디바이스 드라이버의 노드를 생성 ->접근 권한 변경 ->dev에서 드라이버를 접근할수 있도록함 
    printk(KERN_INFO "'sudo mknod /dev/%s c %d 0'\n",GPIO_DEVICE, GPIO_MAJOR);
    printk(KERN_INFO "'sudo chmod 666 /dev/%s'\n",GPIO_DEVICE);
    
    // gpio 권한을 얻어온다
    for(i=0;i<8;i++)
    {
        memset(temp,0,sizeof(temp));
        sprintf(temp,"%d",segs[i]);
        err=gpio_request(segs[i],temp);
    if(err==-EBUSY)
    {
        printk(KERN_INFO "ERROR GPIO_request %d\n",segs[i]);
        return -1;
    }
        gpio_direction_output(segs[i],SEG_LOW);
    }

    //irq 
    /*
    switch_irq[0]=gpio_to_irq(SW_STOP);
    switch_irq[1]=gpio_to_irq(SW_PAUSE);	
	
    err=request_irq(switch_irq[0], irs_stop, IRQF_TRIGGER_RISING,"switch",NULL);
    if(err)
    {
		printk(KERN_INFO "Error request_irq\n");
		return -1;
    }

    err=request_irq(switch_irq[1], isr_pause, IRQF_TRIGGER_RISING,"switch",NULL);
    if(err)
    {
		printk(KERN_INFO "Error request_irq\n");
		return -1;
    }
    */

    //rh timer init
    
    unsigned long delay_in_ms = 100L;
    ktime_t ktime;
    pr_info("HR Timer module installing\n");

    ktime = ktime_set(0, MS_TO_NS(delay_in_ms));
    hrtimer_init( &hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    hr_timer.function = &timer_func;
    pr_info( "Starting timer to fire in %ldms\n",delay_in_ms);

    hrtimer_start(&hr_timer, ktime, HRTIMER_MODE_REL);
    
    

    /*if(!strcmp(cmd,"0"))
	{
		del_timer_sync(&timer);
	}
	else
	{
		init_timer(&timer);
		timer.function=timer_func;
		timer.data=1L;	//timer_func으로 전달하는 인자값
		timer.expires = jiffies + (1*HZ); //1초 뒤에 타이머 만료
		add_timer(&timer);
	}
	if(!strcmp(cmd,"end"))
	{
		pid_valid=0;
	}
	else
	{
		pid_valid=1;
	}*/

    return 0;
}




static void __exit seg_exit(void){//rmmod로 제거 될때 실행 되는 함수
	dev_t devno=MKDEV(GPIO_MAJOR,GPIO_MINOR);
    //gpio repuest에서 받아온 권한을 반납
    int i,ret=0;
     for(i=0;i<8;i++)
     {
        gpio_direction_output(segs[i],SEG_LOW);    
        gpio_free(segs[i]);
     }
     //request_irq에서 받아온 사용권한을 반납한다.
      //free_irq(switch_irq, NULL);
    

    while(ret<=0){
        ret = hrtimer_cancel(&hr_timer);
        pr_info("The timer was still in use...\n");
    }
    pr_info("HR Timer module uninstalling\n");

    // 문자 디바이스의 등록을 해제한다.
   
    unregister_chrdev_region(devno,1);

    // 문자 디바이스의 구조체를 삭제한다.
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
