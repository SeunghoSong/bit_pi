#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <wiringPi.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h> 
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define	LED		1	//GPIO18
#define trig	4	//GPIO23
#define echo	5	//GPIO24

//LED자원을 관리하기 위해 mutex변수를 선언한다.
pthread_mutex_t led_lock;
pthread_mutex_t hc04_lock;

struct timeval UTCtime_r;

void disp_runtime(struct timeval UTCtime_s, struct timeval UTCtime_e)
{
	if ((UTCtime_e.tv_usec - UTCtime_s.tv_usec) < 0)
	{
		UTCtime_r.tv_sec = UTCtime_e.tv_sec - UTCtime_s.tv_sec - 1;
		UTCtime_r.tv_usec = 1000000 + UTCtime_e.tv_usec - UTCtime_s.tv_usec;
	}
	else
	{
		UTCtime_r.tv_sec = UTCtime_e.tv_sec - UTCtime_s.tv_sec;
		UTCtime_r.tv_usec = UTCtime_e.tv_usec - UTCtime_s.tv_usec;
	}
}

void ledControl(int value)
{
	// LED를 위한 MUTEX구현
	if (pthread_mutex_trylock(&led_lock) != EBUSY)
	{
		digitalWrite(LED,1);
        sleep(1);
        digitalWrite(LED,0);
		pthread_mutex_unlock(&led_lock);
	}
	return;
}

void* ledFunction(void *arg)
{
	pinMode(LED, OUTPUT);
	while(1)
    {
    ledControl(1);

    }
    
	
}

void hc04_C()
{
    int startTime, endTime;
	static float distance, before_distance, delta_s;
	double velocity;
	static int flag = 0;
	static int count = 0;
	struct timeval UTCtime_t1, UTCtime_t2;
    if (pthread_mutex_trylock(&hc04_lock) != EBUSY)
	{
        if (!flag && count > 0)
		{	//trigger를 입력하는 시점에서 시간 저정
			gettimeofday(&UTCtime_t1, NULL); // UTC 현재 시간 구하기(마이크로초까지)
			disp_runtime(UTCtime_t2, UTCtime_t1);
			flag = 1;
		}
		else if (flag && count > 0)
		{
			gettimeofday(&UTCtime_t2, NULL); // UTC 현재 시간 구하기(마이크로초까지)
			disp_runtime(UTCtime_t1, UTCtime_t2);
			flag = 0;
		}

		// Trig신호 생성 (10us)
		digitalWrite(trig, HIGH);
		usleep(10);					//wiringPi : delayMicroseconds(10); 
		digitalWrite(trig, LOW);
        
		// Echo back이 '1'이면 시작시간 기록 
		while (digitalRead(echo) == 0);
		//wiringPi : wiringPiSetup이후의 시간을 마이크로초로 측정하는 함수
		startTime = micros();

		// Echo back이 '1'이면 대기 
		while (digitalRead(echo) == 1);
		// Echo back이 '0'이면 종료시간 기록 
		endTime = micros();

		if (count > 0)
			printf("runtime : %ld\n", UTCtime_r.tv_usec);

		// 거리 계산 공식
		before_distance = distance;
		distance = (endTime - startTime)/ 58;
		printf("distance %.2f cm\n", distance);

		if (distance > before_distance)
			delta_s = distance - before_distance;
		else
			delta_s = before_distance - distance;
		printf("delta_s %f \n", delta_s);
		velocity = (delta_s * 10000) / (UTCtime_r.tv_usec);  // unit : m/s
		printf("velocity %f m/s\n", velocity);
		count++;
        pthread_mutex_unlock(&hc04_lock);
    }

}

void* hc04_F(void *arg)
{
    pinMode(trig, OUTPUT);
	pinMode(echo, INPUT);
    digitalWrite(trig, LOW);
    while(1)
    {
        hc04_C();
    }

}


int main(int argc, char **argv)
{
	int err;
	pthread_t thread_LED, thread_HC04;

	//Init
	wiringPiSetup();

	pthread_create(&thread_LED, NULL, ledFunction, NULL);
	pthread_create(&thread_HC04, NULL, hc04_F, NULL);
	pthread_join(thread_LED, 0);
    pthread_join(thread_HC04, 0);

}