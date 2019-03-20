
#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <unistd.h>
#define loopCount 1
#define T 170 //ms
// ./helloGpio [pinNumer]
void pwmled(int flag, int delay);

int main (int argc, char** argv)
{
	int gpioNo;
	int i=0;
	int flag =0;
	wiringPiSetup();//1.wiringPi Init
	//pin numher error
	if(argv<2){
	printf("%s GpioNO \n",argv[0]);
	return -1;
	}

	gpioNo=atoi(argv[1]);
	pinMode(gpioNo,OUTPUT);//2.pin direction
	
	while(1){
	pwmled(1,T);
	pwmled(0,T);	
	
	
	}	
	return 0;
}


void pwmled(int flag,int delay ){
	int i=0;
	if(flag==1){
	i=delay;
	while(i>=0){
	digitalWrite(gpioNo,HIGH);
	usleep(100*(delay-i));
	digitalWrite(gpioNo,LOW);
	usleep(i*100);
	i--;
	}
	}else if (flag==0){
		i=0;
	while(i<delay){
	digitalWrite(gpioNo,HIGH);
	usleep(100*(delay-i));
	digitalWrite(gpioNo,LOW);
	usleep(i*100);
	}
	
	}
}
