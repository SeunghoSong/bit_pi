#include <wiringPi.h>
int a=27;//GPIO16
int b=26;//GPIO12
int c=23;//GPIO13
int d=24;//
int e=25;//GPIO26
int f=28;// GPIO20
int g=29;//GPIO21
int dp=22;//GPIO6
int i; 

void digital_0()//0
{
digitalWrite(a,0);
digitalWrite(b,0);
digitalWrite(c,0);
digitalWrite(d,0);
digitalWrite(e,0);
digitalWrite(f,0);
digitalWrite(g,1);
digitalWrite(dp,1);
}
void digital_1()//1
{
digitalWrite(a,1);
digitalWrite(b,0); 
digitalWrite(c,0); 
digitalWrite(d,1);
digitalWrite(e,1); 
digitalWrite(f,1);
digitalWrite(g,1);
digitalWrite(dp,1); 
}
void digital_2()//2
{
digitalWrite(a,0);
digitalWrite(b,0);
digitalWrite(c,1);
digitalWrite(d,0);
digitalWrite(e,0);
digitalWrite(f,1);
digitalWrite(g,0);
digitalWrite(dp,1);
}
void digital_3()//3
{
digitalWrite(a,0);
digitalWrite(b,0);
digitalWrite(c,0);
digitalWrite(d,0);
digitalWrite(e,1);
digitalWrite(f,1);
digitalWrite(g,0);
digitalWrite(dp,1);
}
void digital_4()//4
{
digitalWrite(a,1);
digitalWrite(b,0);
digitalWrite(c,0);
digitalWrite(d,1);
digitalWrite(e,1);
digitalWrite(f,0);
digitalWrite(g,0);
digitalWrite(dp,1);
}
void digital_5()//5
{
digitalWrite(a,0);
digitalWrite(b,1);
digitalWrite(c,0);
digitalWrite(d,0);
digitalWrite(e,1);
digitalWrite(f,0);
digitalWrite(g,0);
digitalWrite(dp,1);
digitalWrite(e,1);
}
void digital_6()//6
{
digitalWrite(a,0);
digitalWrite(b,1);
digitalWrite(c,0);
digitalWrite(d,0);
digitalWrite(e,0);
digitalWrite(f,0);
digitalWrite(g,0);
digitalWrite(dp,1);
}
void digital_7()//7
{
digitalWrite(a,0);
digitalWrite(b,0);
digitalWrite(c,0);
digitalWrite(d,1);
digitalWrite(e,1);
digitalWrite(f,1);
digitalWrite(g,1);
digitalWrite(dp,1);
}
void digital_8()//8
{
digitalWrite(a,0);
digitalWrite(b,0);
digitalWrite(c,0);
digitalWrite(d,0);
digitalWrite(e,0);
digitalWrite(f,0);
digitalWrite(g,0);
digitalWrite(dp,1);
}
void digital_9()//9
{
digitalWrite(a,0);
digitalWrite(b,0);
digitalWrite(c,0);
digitalWrite(d,0);
digitalWrite(e,1);
digitalWrite(f,0);
digitalWrite(g,0);
digitalWrite(dp,1);
}

void digital_a()//9
{
digitalWrite(a,0);
digitalWrite(b,0);
digitalWrite(c,0);
digitalWrite(d,0);
digitalWrite(e,0);
digitalWrite(f,0);
digitalWrite(g,0);
digitalWrite(dp,0);
}
void digital_b()//9
{
digitalWrite(a,1);
digitalWrite(b,1);
digitalWrite(c,1);
digitalWrite(d,1);
digitalWrite(e,1);
digitalWrite(f,1);
digitalWrite(g,1);
digitalWrite(dp,1);
}
int main()
{
 wiringPiSetup();
{
for(i=22;i<=29;i++)
pinMode(i,OUTPUT); 
 }
 while(1)
  { 
  digital_a();//0
  delay(1000);
  digital_b();//0
  delay(1000);
  } 
}