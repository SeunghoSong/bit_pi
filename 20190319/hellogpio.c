
#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>
#define loopCount 5
#define delayTime 500 //ms
// ./helloGpio [pinNumer]
int main (int argc, char** argv)
{
	int gpioNo;
	int i;
	wiringPiSetup();//1.wiringPi Init
	//pin numher error
	if(argv<2){
	printf("%s GpioNO \n",argv[0]);
	return -1;
	}

	gpioNo=atoi(argv[1]);
	pinMode(gpioNo,OUTPUT);//2.pin direction
	
	//3.pin write
	for(i=0;i<loopCount;i++){
		digitalWrite(gpioNo,HIGH);
		delay(delayTime);
		digitalWrite(gpioNo,LOW);
		delay(delayTime);
	}	
	return 0;
}	
