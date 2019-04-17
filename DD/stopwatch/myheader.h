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

//#define DEBUG
#define BUF_SIZE 100
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

int segs[8]={SEG_A,SEG_B,SEG_C,SEG_D,SEG_E,SEG_F,SEG_G,SEG_DOT};