#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<string.h>
#include<stdlib.h>
#include<sys/stat.h> //sys
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>
#include<dirent.h>
#include<linux/input.h>
#include<sys/select.h>
#include<sys/time.h>
#include<signal.h>
#include<pthread.h>
#include<sys/ioctl.h>
#include<time.h>
#include<sys/mman.h>
#include<math.h>
#define FPGA_BASE_ADDRESS 0x08000000
#define LED_ADDR 0x16
#define FND_DEVICE "/dev/fpga_fnd"
#define MAX_DIGIT 4
#define FPGA_DOT_DEVICE "/dev/fpga_dot"
#define MAX_BUFF 32
#define LINE_BUFF 16
#define FPGA_TEXT_LCD_DEVICE "/dev/fpga_text_lcd"
#define BUFF_SIZE 64
#define KEY_RELEASE 0
#define KEY_PRESS 1
#define MAX_BUTTON 9
struct msgbuf{		//메세지큐 구조체
	long msgtype;
	char mtext[33];	//text값을 넘길 때 사용
	int msg;	//switch, key값을 넘길 때 사용
	int fnd;	//fnd값을 넘길 때 사용
	int led;	//led값을 넘길 때 사용
	unsigned char dot[10];	//dot값을 넘길 때 사용
};
struct player{		//게임 플레이어 구조체
	int x,y,score;	//x,y좌표 점수
};
struct ball{	//공 구조체
	int x,y,z;	//y,x좌표
	int dir;	//진행 방향
};
void* input_key();			//input에서 key값을 넘기는 함수
void* input_switch();		//input에서 switch값을 넘기는 함수
void user_signal1(int sig);	//시그널 확인하는 함수
void init();				//이전에 눌린 알파벳번호를 카운트하는 배열 초기화
void* rcv_msg();			//main에서 넘긴 메세지를 받는 함수
int output_led(int num);	//led 출력 함수
int output_fnd(int num);	//fnd 출력 함수
int output_dot(unsigned char* img);	//dot 출력 함수
int output_lcd(char* text);	//lcd 출력 함수
void* rcvkey_in();			//input이 보낸 key값을 받는함수
void* rcvswit_in();			//input이 보낸 switch값을 받는 함수
void* snd_sec();			//1초에 한번씩 메세지를 보내는 함수
int clockmode(int input);	//clock모드1
int countmode(int input);	//counter모드2
void textmode(int num);		//text editer모드3
void drawmode(int num);		//draw board모드4
void additionalmode(int num);//추가 구현 모드5
int dec_to_otc(int num);	//10진수를 8진수로 바꾸는 함수
int otc_to_quad(int num);	//8진수를 4진수로 바꾸는 함수
int quad_to_bi(int num);	//4진수를 2진수로 바꾸는 함수
int bi_to_dec(int num);		//2진수를 10진수로 바꾸는 함수
int digit(int num,int dig);	//해당 자리수의 숫자를 리턴
int count_input(int num);	//입력받은 switch가 몇개인지 리턴
void reverse();				//그림을 반전 시킴
void draw();				//그림을 그리는 함수
void initialize(int mode);	//모드가 바뀔 때 출력화면 초기화하는 함수
