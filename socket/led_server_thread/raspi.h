#define WR_LED	1
#define RD_HC04	2
#define WR_DIST	3

struct Data
{
	int cmd;
	int endFlag;
	int led_Value;
	float hc04_dist;
};