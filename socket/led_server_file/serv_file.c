/*
---- serv_file.c ----
서버역할. 해당 포트를 입력하여 실행하게 되면 현재 컴퓨터의 ip와 입력된 해당포트로 
client와의 소켓통신을 할수 있도록한다.
여러 client들과의 통신을 위해 쓰레드를 이용해 처리 할수 있도록 했다. 
client의 요청이 담긴 Data 구조체를 통해 
wiringPi를 이용하여 gpio를 통해 LED를 제어하고 
HC04(초음파센서)의 입력값을 받아 물체의 거리를 측정한후
Data 구조체에 정보를 담아 다시 client로 보낸다. 

server->client
led의 상태 정보와 거리 정보를 Data 구조체를 통해 연결된 클라이언트들에게 송신하게 된다.
그리고 클라이언트에게 전송된 사진 데이터를 받고 Data 구조체를 통해 LED 상태를 바꾼다.

client -> server
led의 상태를 바꾸도록 요청한다.
led의 상태 정보와 hc04를 이용한 거리 정보를 요청한다. 
그리고 사진(raspi.jpg)을 전송한다.

-사용방법-

1.라즈 베리 파이 gpio 설정
(-)LED(+)->gpio 18
HC04 설정
hc04-trig 	-> gpio 23
hc04-echo 	-> gpio 24
hc04-VCC 	-> 5v
hc04-GND	-> GND

2.사진 파일 구하기
raspi.jpg 라는 이름으로된 파일을 client_file.c 가 있는 디렉터리에 넣어둔다.

3. serv_file.c 와 client_file 을 컴파일 한다.
$gcc -o serv_file serv_file.c -lwiringPi -lpthread
$gcc -o client_file client_file.c -lwiringPi -lpthread

4.serv_file을 먼저 실행 후 client_file을 실행한다. 
$./serv_file [port number]
(다른 쉘 창 실행)
$./serv_file [ip addr] [port number]
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <wiringPi.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "raspi.h"

#define	LED		1	// GPIO18
#define trig	4	// GPIO23
#define echo	5	// GPIO24

//#define DEBUG
#define MAX_CLNT 256 //최대로 연결이 가능한 클라이언트 수

int clnt_cnt = 0; //소켓파일 디스크립터 파일 배열 카운터
//클라이언트가 접속과 접속해제 처리를 용의하게 하기 위한 소켓파일 디스크립터 배열
int clnt_socks[MAX_CLNT]; 
pthread_mutex_t mutx;

struct Data buf;
struct Data data;

//LED자원을 관리하기 위해 mutex변수를 선언한다.
pthread_mutex_t led_lock;
pthread_mutex_t hc04_lock;

//에러 메세지 표시를 위한 함수
void error_handling(char *message) 
{
	fputs(message, stderr);			
	fputc('\n', stderr);
	exit(1);
}

//=====================================================
// LED Function
//=====================================================
void ledWrite(struct Data *data, int value)
{
	data->led_Value = value;
}

void ledControl(int value)
{
	// LED를 위한 MUTEX구현
	if (pthread_mutex_trylock(&led_lock) != EBUSY)//자원 접근을 위해 접근권한이 있는지확인한다. 
	{
		digitalWrite(LED, value);
		pthread_mutex_unlock(&led_lock);
	}
	return;
}

void* ledFunction(void *arg)
{
	pinMode(LED, OUTPUT);

	while (!data.endFlag)
	{
		ledControl(data.led_Value);
		//printf("led=%d\n", data.led_Value);
		usleep(200000); //200*1000=200ms
	}

}

//=====================================================
// HC_SR04 Function
//=====================================================
float hc04Control(void)
{
	int startTime, endTime;
	float distance;

	// LED를 위한 MUTEX구현
	if (pthread_mutex_trylock(&hc04_lock) != EBUSY)
	{
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

		distance = (endTime - startTime) / 58; //거리 계산
		pthread_mutex_unlock(&hc04_lock); 
		usleep(10000);
		//printf("distance:%f\n", distance);
	}
	return distance;
}

void* hc04Function(void *arg)
{
	float ret; //거리값
	//trig에서 초음파를 쏜후 물체에 반사되어  echo 로 들어오는 초음파를 입력받는다.
	pinMode(trig, OUTPUT); 
	pinMode(echo, INPUT);
	digitalWrite(trig, LOW);
	while (!data.endFlag)
	{
	//trig에서 쏜 시간과 echo에서 받은 시간의 간격과 초음파 속도를 통해 거리값을 측정하게 된다.
		ret = hc04Control();
		data.hc04_dist = ret;
		
	}
	return NULL;
}
//클라이언트가 요청한 작업을 하기 위한 쓰레드
void* userThread(void *arg)
{
	int clnt_sock = *((int*)arg); 
	int str_len = 0, i;
	int read_cnt;
	pthread_t t_id[2];
	FILE *fp;
	char filebuf[BUF_SIZE];
	
	pthread_create(&t_id[0], NULL, ledFunction, 0); //led 처리를 위한 쓰레드
	pthread_create(&t_id[1], NULL, hc04Function, 0); //hc04처리를 위한 쓰레드
	
	//read 함수로 클라이언트의 fd와 연결된 소켓정보로 오는 데이터를 받아온다.
	//성공시 str_len 에 수신된 데이터의 바이트수 실패시에는 -1를 리턴
	//그리고 str_len 에 0을 리턴시에는 해당 소켓으로 들어오는 데이터에 NULL이 들어온것이며 
	//NULL은 파일의 끝,소켓 통신에서는 EOF, EOF는 클라이언트가 접속을 종료했다는 것이다(FIN 수신).
	while ((str_len = read(clnt_sock, &buf, sizeof(buf))) != 0) 
	{
		switch (buf.cmd) //cmd flag를 통해 작업 구분
		{	//LED
			case WR_LED: 
						data.led_Value = buf.led_Value; // 서버측의 data 구조체를 갱신
						printf("data.led_Value=%d\n", data.led_Value);
						break;
			//HC04			
			case RD_HC04:
						data.cmd = WR_DIST; //hc04의 정보를 송신함을 알림
						write(clnt_sock, &data, sizeof(data)); //ACK와 거리정보 client에게 송신
						printf("data.dist:%f\n", data.hc04_dist);
						break;
			//IMAGE
			case RD_IMG: 
						data.cmd = WR_IMG;//이미지 데이터를 받았음을 알림
						write(clnt_sock, &data, sizeof(data)); //ACK
						fp = fopen(FILENAME, "rb"); //read허용 데이터를 읽어올때 바이너리 정보로 읽어옴 (아스키 코드값이 아닐수 있기때문)
						while (1)
						{
							read_cnt = fread((void*)filebuf, 1, BUF_SIZE, fp);
							// 남아있는 파일의 사이즈가 BUF_SIZE보다 작을 경우
							if (read_cnt < BUF_SIZE)
							{
								write(clnt_sock, filebuf, read_cnt);
								break;
							}
							// 남아있는 파일의 사이즈가 BUF_SIZE보다 큰 경우
							write(clnt_sock, filebuf, BUF_SIZE);
						}
						printf("File Write is done\n");
						fclose(fp);
						break;
			default:
						break;
		}
	}

	//클라이언트 접속 종료
	data.endFlag = 1; 

	pthread_detach(t_id[0]); 
	pthread_detach(t_id[1]);
	printf("userThread is end\n");
	pthread_mutex_lock(&mutx); 
	//접속을 종료한 클라이언트의 소켓정보
	for (i = 0; i < clnt_cnt; i++)   // remove disconnected client
	{
		if (clnt_sock == clnt_socks[i])
		{
			while (i++ < clnt_cnt - 1)
				clnt_socks[i] = clnt_socks[i + 1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);


}

static void sigHandler(int signum)
{
	printf("sigHanlder\n");
	exit(0);
}

int main(int argc, char **argv)
{
	int err; 
	int serv_sock, clnt_sock; //소켓 파일 디스크립터 번호를 저장하기 위함
	pthread_t thread_LED; 		
	pthread_t t_id;
	int str_len;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	data.endFlag = 0;
	//Init wiringPi를 사용을 위함
	wiringPiSetup();
	// SIGINT(ctrl +c)에 대한 시그널 처리를 위함
	signal(SIGINT, sigHandler);
	
	// STEP 1. socket생성
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	//STEP 2. bind : 사용할 포트와 IP를 등록 
	if (bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1){
		error_handling("bind() error");

	//STEP 3. 포트 연결을 허용 그리고 클라이언트의 연결 요청을 처리하기 위한  대기 큐를 5개로 설정
	if (listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	while (1)
	{	
		clnt_adr_sz = sizeof(clnt_adr);
		//accpt 연결 요청 처리
		//서버가 허용한 ip와 포트로 연결 요청이 들어올때까지 대기 (blocking)
		//클라이언트에게 연결요청이 왔을경우 전송된 클라이언트의 소켓정보를  clnt_adr에 저장하고 
		//그 구조체와 연결된 파일 디스크립터를 리턴한다.
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

		//해당 자원을 뮤텍스로 접근을 막는다.
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++] = clnt_sock; //클라이언트의 소켓 fd 번호 저장
		pthread_mutex_unlock(&mutx);

		// 사용자의 요청에 대응하기 위해 클라이언트당 1개의 스레드 생성 (led,hc04,file 전송 처리)
		pthread_create(&t_id, NULL, userThread, (void*)&clnt_sock);
		pthread_detach(t_id); // blocking 없이 좀비 쓰레드 상태가 될시 자원 반납을 한후 쓰레드 종료
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr)); //연결 한 client의 ip를 출력
	}

#ifdef DEBUG
	// 회로 연결을 테스트 하기 위한 코드
	while (1)
	{
		if (data.led_Value == 1)
			ledWrite(&data, LOW);
		else
			ledWrite(&data, HIGH);
		sleep(1);
	}
#endif
	   
	close(serv_sock);
	return 0;

}