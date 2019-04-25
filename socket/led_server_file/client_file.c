/* 							 
---- client_file.c ----
클라이언트 역할. 소켓 통신을 할 서버의 ip주소와 포트 번호를 입력하여 실행하게 되면 
해당 ip 서버와 소켓통신을 시작하게된다.
클라이언트는 서버에게 LED 정보와 HC04를 이용한 거리 측정 데이터 송신을 요청한다.
그리고 "raspi.jpg" 라는 파일을 서버에게 전송하게 된다.
서버 측에서 전송한 Data 구조체를 받아 출력한다.



server->client
클라이언트에게 전송된 사진 데이터를 받고 Data 구조체를 통해 LED 상태를 바꾼다.
led의 상태 정보와 거리 정보를 Data 구조체를 통해 연결된 클라이언트들에게 송신하게 된다.


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

3 serv_file.c 와 client_file 을 컴파일 한다.
$gcc -o serv_file serv_file.c -lwiringPi -lpthread
$gcc -o client_file client_file.c -lwiringPi -lpthread

4.serv_file을 먼저 실행 후 client_file을 실행한다. 
$./serv_file [port number]
(다른 쉘 창 실행)
$./client_file [ip addr] [port number]
*/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "raspi.h"

void error_handling(char *message);
void read_routine(int sock, char *buf);
void write_routine(int sock, char *buf);

struct Data data;

int main(int argc, char *argv[])
{
	int sock;
	pid_t pid;
	char buf[BUF_SIZE];
	struct sockaddr_in serv_adr;


	if(argc!=3) { //연결할 ip 정보와 포트를 입력받아야한다.
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	//클라이언트 소켓 정보 생성
	sock=socket(PF_INET, SOCK_STREAM, 0);  
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));
	// 해당 소켓 정보를 통해 서버와의 소켓 통신을 연결 시도
	if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("connect() error!");
	//멀티 프로세서 생성
	pid=fork();
	if(pid==0)
		write_routine(sock, buf); //자식 프로세서
	else 
		read_routine(sock, buf); //부모 프로세서

	close(sock);
	return 0;
}
//server->client (수신 데이터)
void read_routine(int sock, char *buf)
{
	int read_cnt;
	char filebuf[BUF_SIZE];
	FILE *fp;

	while(1)
	{
		int str_len=read(sock, &data, sizeof(data));
		if(str_len==0) //전송된 데이터가 끝남 서버와의 연결이 종료됨 NULL EOF FIN 
			return;
		switch (data.cmd)
		{
			// HC04의 거리데이터를 수신하면 값을 출력한다.
			case WR_DIST: printf("data.hc04_dist=%f\n", data.hc04_dist);
				break;
			case WR_IMG: 
						fp = fopen("result.jpg", "w+b"); //쓰기와 덮어쓰기 허용 그리고 바이너리데이터로 읽어오기
						while ((read_cnt = read(sock, filebuf, BUF_SIZE)) != 0)
						{
							fwrite((void*)filebuf, 1, read_cnt, fp);
							if(read_cnt<BUF_SIZE) //전송데이터가 버퍼 사이즈 보다 작을경우
								break; 				//끝낸다. 실제 사진 크기를 넘겨 파일받는 경우를 방지
						}
						fclose(fp);
						printf("File Write is done\n");
						break;
		}
	}
}
//clinet ->server (송신 데이터)
void write_routine(int sock, char *buf)
{
	data.led_Value = 0;

	while (1)
	{
		// LED를 깜빡이기 위한 코드
		if (data.led_Value == 1)
			data.led_Value = 0;
		else
			data.led_Value = 1;

		// 서버에 LED값 변경을 위한 요청 데이터 보내기
		data.cmd = WR_LED;
		printf("data.led_Value=%d\n", data.led_Value);
		write(sock, &data, sizeof(data));

		// 서버에 HC04 거리 요청 데이터 보내기
		data.cmd = RD_HC04;
		write(sock, &data, sizeof(data));
		
		data.cmd = RD_IMG;
		write(sock, &data, sizeof(data));


		sleep(10);
	}

}
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}