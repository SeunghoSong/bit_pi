/*커널 스레드 생성



!!!!!!! 작성시 주의 !!!!!! */


/*
 insmod()를 호출해서 모듈을 커널에 적재
 rmmod()를 호출해서 모듈을 커널에서 제거
 
 lsmod 명령어로는 kernel에 적재된 모듈들을 볼수 있다.
 dmesg 명령어로는 kernel에 대한 log 기록을 볼수 있다.
 
 */


//커널 작성시 필요한 헤더파일
#include <linux/init.h> // 초기화시 필요
#include <linux/module.h>//
#include <linux/kernel.h>


//hello_init(exit) 함수 작성
static int __init hello_init(void){

	printk("called hello_init()\n"); //kernel log에 출력된다.
	return 0;
	}

static void __exit hello_exit(void){
	printk("called hello_exit()\n");
	}

//커널 작성시 반드시 들어가야하는 함수들 

module_init(hello_init);//init(초기화)할때 hello_init 함수 호출
module_exit(hello_exit);//exit(종료)할때

MODULE_LICENSE("GPL");//  license 작성(GPL,GPLv2)  안할시에 오류 또는 warnning
MODULE_AUTHOR("Seung ho Song"); // 제작자 이름
MODULE_DESCRIPTION("HELLO MODULE"); //용도
