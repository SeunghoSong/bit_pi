//cmd flag 들
#define WR_LED	1
#define RD_HC04	2
#define WR_DIST	3
#define RD_IMG	4
#define WR_IMG	5

#define FILENAME "raspi.jpg" //파일 이름
#define BUF_SIZE 1024		// 바퍼 사이즈

struct Data
{
	int cmd;     
	int endFlag;  			//client 종료가 확인 되었을시에 1 아닐시 0
	int led_Value;			//led 상태정보 
	float hc04_dist;		//거리 정보
};