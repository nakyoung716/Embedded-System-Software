#include"total.h"
unsigned char fpga_number[3][10]={{0,0,0,0,0,0,0,0,0,0},{0x0c,0x1c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x3f,0x3f}, {0x1c,0x36,0x63,0x63,0x63,0x7f,0x7f,0x63,0x63,0x63}};
unsigned char dot[10][7];
int x=0;
int y=0;
int mode=0;
int hour=0;
int minute=0;
int cmode=0;
int lednum=128;
int fndnum=0;
int fcount=0;
int fmode=0;
int tmode=0;
int prev=0;
int cursor=1;
int textn=0;
int cnt[9]={0,0,0,0,0,0,0,0,0};
int start=0;
int end=0;
struct player p1,p2;
struct ball ball1;
int pid1,pid2;
key_t key_id1,key_id2,key_id3;
struct msgbuf mybuf1,mybuf2,mybuf3;
int main()
{
	int rcv_msg;
	pthread_t p_thread[3];
	int thr_id;
	key_id3 = msgget((key_t)4321,IPC_CREAT|0666);	//메세지큐 생성
	if(key_id3==-1)
	{
		perror("msgget error 1: ");
		exit(0);
	}
	memset(mybuf3.mtext,0x00,33);
	strcpy(mybuf3.mtext,"");
	mybuf3.msg=0;
	mybuf3.fnd=0;
	mybuf3.led=0;
	memset(mybuf3.dot,0,10);//메세지큐 내용 초기화
	pid1 =fork();	//fork
	if(pid1<0)
	{
		printf("fork1 failed\n");
	}
	else if(pid1==0) //input process
	{
		in_main();
	}
	else
	{
		pid2=fork();	//fork
		if(pid2<0)
		{
			printf("fork2 failed\n");
		}
		else if(pid2==0) //output process
		{
			out_main();	
		}
		else //parent process
		{
			thr_id=pthread_create(&p_thread[0],NULL,rcvkey_in,NULL);	//input에서 보내는 key시그널 받는 함수에 대한 쓰레드 생성
			thr_id=pthread_create(&p_thread[1],NULL,rcvswit_in,NULL);	//input에서 보내는 switch시그널 받는 함수에 대한 쓰레드 생성
			thr_id=pthread_create(&p_thread[2],NULL,snd_sec,NULL);		//main에서 1초에 한번씩 시그널 보내는 함수에 대한 쓰레드 생성
			pthread_join(p_thread[0],(void**)NULL);
			pthread_join(p_thread[1],(void**)NULL);
			pthread_join(p_thread[2],(void**)NULL);
		}
	}	
	return 0;
}
void* snd_sec(){	//main에서 1초에 한번씩 메세지 전달
	int i=0,j=0,tmp_hour,tmp_min;
	time_t timer;
	struct tm *t;
	while(1)
	{
		timer=time(NULL);
		t=localtime(&timer);
		if(mode==0)
		{
			mybuf3.msgtype=3;
			if(cmode==0)	//clock 모드의 시간이 변경모드가 아닐 때
			{
				lednum=128;
				mybuf3.led=lednum;
				if(i/60>0)		//시간 설정 후 1분이 지나면 분을 증가
				{
					i=0;
					tmp_min=(fndnum+1)%100;
					tmp_hour=fndnum/100;
					if(tmp_min>=60)		//분이 60이 되면 0으로 초기화 후 시간을 증가
					{
						tmp_min-=60;
						tmp_hour++;
					}
					if(tmp_hour>=24)
						tmp_hour-=24;
					fndnum=tmp_hour*100+tmp_min;
				}
				mybuf3.fnd=fndnum;		//현재시간 메세지에 저장
				//printf("fnd %d\n",mybuf3.fnd);
			}
			else	//clock모드의 시간을 변경할 수 있을 때
			{
				if(lednum==32)	//led 3번과 4번이 번갈아 메세지에 저장
					lednum=16;
				else
					lednum=32;
				mybuf3.led=lednum;
				mybuf3.fnd=fndnum;
				i=0;
			}
			if(msgsnd(key_id3,(void *)&mybuf3,sizeof(struct msgbuf), IPC_NOWAIT)==-1)	//메세지를 output에 전달
			{
				perror("msgsnd error : ");
				exit(0);
			}
			printf("sec %d\n",i);
			sleep(1);		//전체 구조가 1초마다 반복
			i++;
		}
		else if(mode==3)	//draw모드일 때
		{
			if(cursor==1&&prev==0)	//커서가 켜져있고, 불이 안들어와 있다면
			{
				dot[y][x]=1-dot[y][x];	//깜빡거린다
				draw();
				if(msgsnd(key_id3,(void *)&mybuf3,sizeof(struct msgbuf), IPC_NOWAIT)==-1)	//메세지를 output에 전달
				{
					perror("msgsnd error : ");
					exit(0);
				}
				sleep(1);
				//usleep(1000*100);
			}
		}
		else if(mode==4&&start&&!end)	//모드4 게임 시작하고 게임 종료가 안되었을 때
		{
			//usleep(1000);
			dot[ball1.y][ball1.z]=1-dot[ball1.y][ball1.z];
			if(j%2==1){
				switch(ball1.dir)	//볼의 이동방향
				{
					case 1:	//진행방향이 1사분면
						if(ball1.z!=6&&dot[ball1.y-1][ball1.z+1]!=1)	//직진
						{
							ball1.z++;
							ball1.y--;
						}
						else if(ball1.z!=6&&dot[ball1.y-1][ball1.z+1]==1)	//튕김
						{
							ball1.z++;
							ball1.y++;
							ball1.dir=4;
						}
						else if(ball1.z==6&&dot[ball1.y-1][ball1.z-1]==1)	//튕김
						{
							ball1.z--;
							ball1.y++;
							ball1.dir=3;
						}
						else if(ball1.z==6&&dot[ball1.y-1][ball1.z]!=1)	//튕김
						{
							ball1.z--;
							ball1.y--;
							ball1.dir=2;
						}
						break;
					case 2:	//진행방향이 2사분면
						if(ball1.z!=0&&dot[ball1.y-1][ball1.z-1]!=1)  //직진
						{
							ball1.z--;
							ball1.y--;
						}
						else if(ball1.z!=0&&!(ball1.y!=0&&dot[ball1.y-1][ball1.z-1]!=1)) //튕김
						{
							ball1.z--;
							ball1.y++;
							ball1.dir=3;
						}
						else if(ball1.z==0&&dot[ball1.y-1][ball1.z+1]==1)	//튕김
						{
							ball1.z++;
							ball1.y++;
							ball1.dir=4;
						}
						else if(ball1.z==0&&dot[ball1.y-1][ball1.z-1]!=1)	//튕김
						{
							ball1.z++;
							ball1.y--;
							ball1.dir=1;
						}
						break;
					case 3:	//진행방향이 3사분면
						if(ball1.z!=0&&dot[ball1.y+1][ball1.z-1]!=1)	//직진
						{
							ball1.z--;
							ball1.y++;
						}
						else if(ball1.z!=0&&dot[ball1.y+1][ball1.z-1]==1) //튕김
						{
							ball1.z--;
							ball1.y--;
							ball1.dir=2;
						}
						else if(ball1.z==0&&dot[ball1.y+1][ball1.z+1]==1)	//튕김
						{
							ball1.z++;
							ball1.y--;
							ball1.dir=1;
						}
						else if(ball1.z==0&&dot[ball1.y+1][ball1.z-1]!=1)	//튕김
						{
							ball1.z++;
							ball1.y++;
							ball1.dir=4;
						}
						break;
					case 4:	//진행방향이 4사분면
						if(ball1.z!=6&&dot[ball1.y+1][ball1.z+1]!=1)	//직진
						{
							ball1.z++;
							ball1.y++;
						}
						else if(ball1.z!=6&&dot[ball1.y+1][ball1.z+1]==1)	//튕김
						{
							ball1.z++;
							ball1.y--;
							ball1.dir=1;
						}
						else if(ball1.z==6&&dot[ball1.y+1][ball1.z-1]==1)	//튕김
						{
							ball1.z--;
							ball1.y--;
							ball1.dir=2;
						}
						else if(ball1.z==6&&dot[ball1.y+1][ball1.z]!=1)		//튕김
						{
							ball1.z--;
							ball1.y++;
							ball1.dir=3;
						}
						break;
				}
				if(ball1.y==0)	//공이 가장 위로 올라가면 player2의 득점
				{
					p2.score++;
					ball1.y=4;
					ball1.z=3;
					ball1.dir=1;
				}
				else if(ball1.y==9)	//공이 가장 위로 올라가면 player1의 득점
				{
					p1.score++;
					ball1.y=4;
					ball1.z=3;
					ball1.dir=1;
				}
				if(p1.score>=5||p2.score>=5)
					end=1;
				fndnum=p1.score+p2.score*100;	//현재 스코어 출력   p2/p1형태
				mybuf3.fnd=fndnum;
			}
			draw();
			if(!end)
				mybuf3.led=(pow(2,p1.score)-1)*16+pow(2,p2.score)-1;	//현재 스코어 led 표시
			if(msgsnd(key_id3,(void *)&mybuf3,sizeof(struct msgbuf), IPC_NOWAIT)==-1)	//메세지를 output에 전달
			{
				perror("msgsnd error : ");
				exit(0);
			}
			j++;
			usleep(500*1000);
		}
	}
}
void* rcvkey_in(){	//input에서 넘긴 key값을 받는 함수
	int msgtype;
	msgtype = 3;

	key_id1 = msgget(1234, IPC_CREAT|0666);		//key 메세지 생성
	if (key_id1 < 0)
	{
		perror("msgget error 4: ");
		exit(0);
	}
	while(1)
	{
		if (msgrcv( key_id1, (void *)&mybuf1, sizeof(struct msgbuf), msgtype,0 ) == -1)		//input에서 키를 넘길때까지 대기
		{
			perror("msgrcv error2 : ");
			exit(0);    
		}
		printf("receive from input1\n");
		printf("%d\n",mybuf1.msg);
		if(mybuf1.msg==115)	//볼륨 업이 눌렷을 때 모드를 1 증가
			mode++;
		else if(mybuf1.msg==114)	//볼륨 다운이 눌렷을 때 모드를 1 감소
			mode--;
		else if(mybuf1.msg==158)	//back이 눌렸을 때 프로그램 종료
		{
			mybuf3.fnd=0;
			mybuf3.led=0;
			memset(mybuf3.dot,0,10);
			strcpy(mybuf3.mtext,"");	//출력화면 전부 초기화
			if(msgsnd( key_id3, (void *)&mybuf3, sizeof(struct msgbuf), IPC_NOWAIT )==-1)
			{
				perror("msgsnd error : ");
			}
			usleep(100*1000);
			kill(pid2,9);		//프로세스 종료
			kill(pid1,9);
			exit(0);
		}
		if(mode==5)		//모드가 끝까지 올라가면 처음으로
			mode=0;
		if(mode==-1)	//모드가 끝까지 내려가면 마지막으로
			mode=4;
		switch (mode) {
			case 0:
				initialize(0);	//각종 전역변수들을 초기화 해주는 함수
				break;
			case 1:
				initialize(1);
				break;
			case 2:
				initialize(2);
				break;
			case 3:
				initialize(3);
				break;
			case 4:
				initialize(4);
				break;
			default :
				printf("Error rcvkey\n");
				break;
		}
		mybuf3.msgtype=3;
		if(msgsnd(key_id3,(void *)&mybuf3,sizeof(struct msgbuf), IPC_NOWAIT)==-1)	//main에서 output에 메세지를 전달
		{
			perror("msgsnd error : ");
			exit(0);
		}
		printf("send mode %d\n",mode);
	}
	//exit(0);
	return ;
}
void* rcvswit_in(){	//input에서 넘긴 switch값을 받는 함수
	int msgtype,i;
	msgtype = 4;

	key_id2 = msgget(5678, IPC_CREAT|0666);	//switch 메세지 생성
	if (key_id2 < 0)
	{
		perror("msgget error 5: ");
		exit(0);
	}
	clockmode(0);	//처음 시작할 때 현재시간을 넘겨준다.
	mybuf3.msgtype=3;
	if(msgsnd(key_id3,(void *)&mybuf3,sizeof(struct msgbuf), IPC_NOWAIT)==-1)	//현재시간을 받았으니 메세지를 한 번 넘겨준다
	{
		perror("msgsnd error : ");
		exit(0);
	}
	while(1)	//무한루프를 돌며 switch값을 전달받는다.
	{
		//printf("***%d***%d\n",ball1.y,ball1.x);
		if (msgrcv( key_id2, (void *)&mybuf2, sizeof(struct msgbuf), msgtype,0 ) == -1)
		{
			perror("msgrcv error : ");
			exit(0);    
		}
		printf("receive from input2\n");
		printf("%d\n",mybuf2.msg);
		switch (mode){
			case 0:		//clock모드일 때
				if(count_input(mybuf2.msg)==1)	//전달받은 스위치가 1개일 때
				{
					if(digit(mybuf2.msg,9)==1)	//1번 스위치가 눌렷을 경우
						clockmode(1);
					else if(digit(mybuf2.msg,8)==1)	//2번 스위치가 눌렷을 경우
						clockmode(2);
					else if(digit(mybuf2.msg,7)==1)	//3번 스위치가 눌렷을 경우
						clockmode(3);
					else if(digit(mybuf2.msg,6)==1)	//4번 스위치가 눌렷을 경우
						clockmode(4);
				}
				break;
			case 1:		//count모드일 때
				if(count_input(mybuf2.msg)==1)	//스위치가 1개 눌렷을 때
				{
					if(digit(mybuf2.msg,9)==1)	//1번 스위치가 눌렷을 경우
						countmode(1);
					else if(digit(mybuf2.msg,8)==1)	//2번 스위치가 눌렷을 경우
						countmode(2);
					else if(digit(mybuf2.msg,7)==1)	//3번 스위치가 눌렷을 경우
						countmode(3);
					else if(digit(mybuf2.msg,6)==1)	//4번 스위치가 눌렷을 경우
						countmode(4);
				}
				break;
			case 2:		//text 모드일 때
				textmode(mybuf2.msg);
				break;
			case 3:		//draw 모드일 때
				if(count_input(mybuf2.msg)==1)	//스위치가 한개 눌린경우
					drawmode(mybuf2.msg);
				break;
			case 4:		//추가 구현 모드
				additionalmode(mybuf2.msg);
				break;
			default:
				printf("Error rcvswit\n");
				break;
		}
		mybuf3.msgtype=3;
		if(msgsnd(key_id3,(void *)&mybuf3,sizeof(struct msgbuf), IPC_NOWAIT)==-1)	//main에서 output에 메세지를 보낸다
		{
			perror("msgsnd error : ");
			exit(0);
		}
		printf("send fnd %d\n",mybuf3.fnd);
	}

	//exit(0);
	return ;
}
int clockmode(int input)	//clock 모드1
{
	time_t timer;
	struct tm *t;
	int tmp_hour,tmp_min;
	tmp_hour=fndnum/100;
	tmp_min=fndnum%100;
	switch (input){	
		case 0 :	//모드1의 초기화면
			timer=time(NULL);
			t=localtime(&timer);
			tmp_hour=t->tm_hour;
			tmp_min=t->tm_min;
			break;
		case 1 :	//1번 스위치가 눌린경우
			cmode++;	//변경모드로 전환
			if(cmode>=2)
			{
				lednum=128;
				cmode=0;

			}
			break;
		case 2 :	//2번 스위치가 눌린경우
			if(cmode==1)	//현재 보드 시간
			{
				timer=time(NULL);
				t=localtime(&timer);
				tmp_hour=t->tm_hour;
				tmp_min=t->tm_min;
			}
			break;
		case 3 :	//3번 스위치가 눌린경우
			if(cmode==1)	//변경모드이면
			{
				tmp_hour+=1;	//시간 증가
				if(tmp_hour>=24)
					tmp_hour-=24;
			}
			break;
		case 4 : //4번 스위치가 눌린경우
			if(cmode==1)	//변경모드이면
			{
				tmp_min++;	//분 증가
				if(tmp_min>=60)
				{
					tmp_min-=60;
					tmp_hour++;
				}
				if(tmp_hour>=24)
					tmp_hour-=24;
			}
			break;
		default: break;
	}
	fndnum=tmp_hour*100+tmp_min;
	mybuf3.fnd=fndnum;
	mybuf3.led=lednum;
}
int countmode(int input)	//counter 모드 2
{
	switch(input){
		case 1:
			fmode++;
			if(fmode>=4)
				fmode=0;
			if(fmode==1)
				fndnum=dec_to_otc(fndnum);	//10진수에서 8진수로
			else if(fmode==2)
				fndnum=otc_to_quad(fndnum);	//8진수에서 4진수로
			else if(fmode==3)
				fndnum=quad_to_bi(fndnum);	//4진수에서 2진수로
			else if(fmode==0)
				fndnum=bi_to_dec(fndnum);	//2진수에서 10진수로
			break;
		case 2:
			fndnum+=100;		//백의 자리수 증가
			break;
		case 3:
			fndnum+=10;			//십의 자리수 증가
			break;
		case 4:
			fndnum+=1;			//일의 자리수 증가
			break;
		default:
			break;
	}
	printf("fndnum %d\n",fndnum);
	switch(fmode){
		case 0:	//십진수일 때
			if(fndnum>=1000)	//천의 자리 나오지 않게 처리
				fndnum%=1000;
			lednum=64;	//2번째 led
			break;
		case 1:
			if(digit(fndnum,1)==8)	//각 자리수 진법에 맞게 처리
				fndnum+=2;
			if(digit(fndnum,2)==8)
				fndnum+=20;
			if(digit(fndnum,3)==8)
				fndnum+=200;
			if(fndnum>=800)		//천의 자리 나오지 않게 처리
				fndnum-=1000;
			lednum=32;	//3번째 led
			break;
		case 2:
			if(digit(fndnum,1)==4)	//각 자리수 진법에 맞게 처리
				fndnum+=6;
			if(digit(fndnum,2)==4)
				fndnum+=60;
			if(digit(fndnum,3)==4)
				fndnum+=600;
			if(fndnum>=400)		//천의 자리 나오지 않게 처리
				fndnum-=1000;
			lednum=16;	//4번째 led
			break;
		case 3:
			if(digit(fndnum,1)==2)	//각 자리수 진법에 맞게 처리
				fndnum+=8;
			if(digit(fndnum,2)==2)
				fndnum+=80;
			if(digit(fndnum,3)==2)
				fndnum+=800;
			if(fndnum>=200)		//천의 자리 나오지 않게 처리
				fndnum-=1000;
			lednum=128;	//1번째 led
			break;
		default:
			break;
	}
	mybuf3.fnd=fndnum;
	mybuf3.led=lednum;
}
void textmode(int num)	//text editor 모드3
{
	char temp[2];
	int i;
	if(tmode==0)	//알파벳모드
	{
		if(count_input(num)==1)		//스위치가 1개 눌렸을 때
		{
			if(strlen(mybuf3.mtext)>=32&&cnt[find_switch(num)-1]==0)	//32개가 이미 저장되어 있다면 한칸씩 땡기고 다음 문자를 넣는다
			{
				for(i=0;i<32;i++)
					mybuf3.mtext[i]=mybuf3.mtext[i+1];
				mybuf3.mtext[32]='\0';
			}
			switch (find_switch(num)){	//번호에 해당하는 값을 문자열에 추가
				case 1:
					if(cnt[0]==0)
					{
						init();
						strcat(mybuf3.mtext,".");
						cnt[0]++;
					}
					else if(cnt[0]==1)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='Q';
						cnt[0]++;
					}
					else if(cnt[0]==2)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='Z';
						cnt[0]++;
					}
					else if(cnt[0]==3)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='.';
						cnt[0]=1;
					}
					break;
				case 2:
					if(cnt[1]==0)
					{
						init();
						strcat(mybuf3.mtext,"A");
						cnt[1]++;
					}
					else if(cnt[1]==1)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='B';
						cnt[1]++;
					}
					else if(cnt[1]==2)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='C';
						cnt[1]++;
					}
					else if(cnt[1]==3)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='A';
						cnt[1]=1;
					}
					break;
				case 3:
					if(cnt[2]==0)
					{
						init();
						strcat(mybuf3.mtext,"D");
						cnt[2]++;
					}
					else if(cnt[2]==1)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='E';
						cnt[2]++;
					}
					else if(cnt[2]==2)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='F';
						cnt[2]++;
					}
					else if(cnt[2]==3)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='D';
						cnt[2]=1;
					}
					break;
				case 4:
					if(cnt[3]==0)
					{
						init();
						strcat(mybuf3.mtext,"G");
						cnt[3]++;
					}
					else if(cnt[3]==1)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='H';
						cnt[3]++;
					}
					else if(cnt[3]==2)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='I';
						cnt[3]++;
					}
					else if(cnt[3]==3)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='G';
						cnt[3]=1;
					}
					break;
				case 5:
					if(cnt[4]==0)
					{
						init();
						strcat(mybuf3.mtext,"J");
						cnt[4]++;
					}
					else if(cnt[4]==1)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='K';
						cnt[4]++;
					}
					else if(cnt[4]==2)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='L';
						cnt[4]++;
					}
					else if(cnt[4]==3)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='J';
						cnt[4]=1;
					}
					break;
				case 6:
					if(cnt[5]==0)
					{
						init();
						strcat(mybuf3.mtext,"M");
						cnt[5]++;
					}
					else if(cnt[5]==1)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='N';
						cnt[5]++;
					}
					else if(cnt[5]==2)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='O';
						cnt[5]++;
					}
					else if(cnt[5]==3)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='M';
						cnt[5]=1;
					}
					break;
				case 7:
					if(cnt[6]==0)
					{
						init();
						strcat(mybuf3.mtext,"P");
						cnt[6]++;
					}
					else if(cnt[6]==1)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='R';
						cnt[6]++;
					}
					else if(cnt[6]==2)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='S';
						cnt[6]++;
					}
					else if(cnt[6]==3)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='P';
						cnt[6]=1;
					}
					break;
				case 8:
					if(cnt[7]==0)
					{
						init();
						strcat(mybuf3.mtext,"T");
						cnt[7]++;
					}
					else if(cnt[7]==1)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='U';
						cnt[7]++;
					}
					else if(cnt[7]==2)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='V';
						cnt[7]++;
					}
					else if(cnt[7]==3)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='T';
						cnt[7]=1;
					}
					break;
				case 9:
					if(cnt[8]==0)
					{
						init();
						strcat(mybuf3.mtext,"W");
						cnt[8]++;
					}
					else if(cnt[8]==1)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='X';
						cnt[8]++;
					}
					else if(cnt[8]==2)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='Y';
						cnt[8]++;
					}
					else if(cnt[8]==3)
					{
						mybuf3.mtext[strlen(mybuf3.mtext)-1]='W';
						cnt[8]=1;
					}
					break;
				default:
					break;
			}
			fndnum++;	//버튼 누른 횟수
		}
	}
	else if(tmode==1)	//숫자모드
	{
		if(count_input(num)==1)	//스위치가 두개 눌렸을 때
		{
			if(strlen(mybuf3.mtext)>=32)	//이미 32개의 문자가 출력된다면 한칸씩 당기고 마지막에 문자를 추가한다
			{
				for(i=0;i<32;i++)
					mybuf3.mtext[i]=mybuf3.mtext[i+1];
				mybuf3.mtext[32]='\0';
			}
			switch (find_switch(num)){	//해당 숫자를 문자열에 추가
				case 1:
					strcpy(temp,"1");
					break;
				case 2:
					strcpy(temp,"2");
					break;
				case 3:
					strcpy(temp,"3");
					break;
				case 4:
					strcpy(temp,"4");
					break;
				case 5:
					strcpy(temp,"5");
					break;
				case 6:
					strcpy(temp,"6");
					break;
				case 7:
					strcpy(temp,"7");
					break;
				case 8:
					strcpy(temp,"8");
					break;
				case 9:
					strcpy(temp,"9");
					break;
			}
			strcat(mybuf3.mtext,temp);
			fndnum++;
		}
	}
	if(count_input(num)==2)	//스위치가 두개 눌린경우
	{
		if(digit(num,4)==1&&digit(num,5)==1)	//5,6이 눌리면
		{
			tmode++;		//모드를 바꿔준다
			if(tmode==2)
			{
				tmode=0;
				memcpy(mybuf3.dot,fpga_number[2],10);
			}
			else
				memcpy(mybuf3.dot,fpga_number[1],10);
			fndnum++;
			init();
		}
		else if(digit(num,7)==1&&digit(num,8)==1)	//2,3이 눌리면
		{
			strcpy(mybuf3.mtext,"");	//문자열이 초기화 된다
			fndnum++;
		}
		else if(digit(num,1)==1&&digit(num,2)==1)	//8,9가 눌리면
		{
			if(strlen(mybuf3.mtext)>=32)
			{
				for(i=0;i<32;i++)
					mybuf3.mtext[i]=mybuf3.mtext[i+1];
				mybuf3.mtext[32]='\0';
			}
			strcat(mybuf3.mtext," ");	//공백을 입력
			init();
			fndnum++;
		}
	}
	mybuf3.fnd=fndnum%10000;		//4자리 수까지만 출력
	printf("%s\n",mybuf3.mtext);
}
void drawmode(int num)	//draw board모드4
{
	switch (find_switch(num)){		//해당 스위치를 누르면
		case 1:
			x=0;
			y=0;
			cursor=1;
			prev=0;
			memset(dot,0,70);	//모두 초기화
			break;
		case 2:
			if(y>0)
			{
				dot[y][x]=prev;
				y--;			//위로 한칸
				prev=dot[y][x];	
			}
			break;
		case 3:
			cursor=1-cursor;	//커서 온/오프
			dot[y][x]=prev;
			break;
		case 4:
			if(x>0)
			{
				dot[y][x]=prev;
				x--;			//왼쪽으로 한칸
				prev=dot[y][x];
			}
			break;
		case 5:
			dot[y][x]=1-prev;	//불 온/오프
			prev=dot[y][x];
			break;
		case 6:
			if(x<6)
			{
				dot[y][x]=prev;
				x++;			//오른쪽으로 한칸
				prev=dot[y][x];
			}
			break;
		case 7:
			memset(dot,0,70);	//그림만 초기화
			prev=0;
			break;
		case 8:
			if(y<9)
			{
				dot[y][x]=prev;
				y++;			//밑으로 한칸	
				prev=dot[y][x];
			}
			break;
		case 9:
			reverse();			//반전
			prev=1-prev;
			break;
	}
	fndnum++;
	mybuf3.fnd=fndnum%10000;	//4자리까지만 저장
	draw();	//그리기
}
void additionalmode(int num)	//추가구현 모드 게임 "pong"구현
{
	int i,j;
	for(j=0;j<9;j++)
	{
		if(digit(num,j+1)==1&&count_input(num)<=2) //스위치가 두개이하로 눌렷을 때
		{
			switch(9-j){	//눌린 스위치를 수행
				case 1:		//player1의 막대 좌로 이동
					if(p1.x-1!=0)
						p1.x--;
					break;
				case 2:		//player1의 막대 상/하로 이동
					p1.y=1-p1.y;
					break;
				case 3:		//player1의 막대 우로 이동
					if(p1.x+1!=6)
						p1.x++;
					break;
				case 4:		//이전 텍스트
					if(textn!=0)
						textn--;
					break;
				case 5:		//게임 시작
					start=1;
					break;
				case 6:		//이후 텍스트
					if(textn!=7)
						textn++;
					break;
				case 7:		//player2의 막대 좌로 이동
					if(p2.x-1!=0)
						p2.x--;
					break;
				case 8:		//player2의 막대 상/하로 이동
					p2.y=17-p2.y;
					break;
				case 9:		//player2의 막대 우로 이동
					if(p2.x+1!=6)
						p2.x++;
					break;
				default:
					break;
			}
			for(i=0;i<7;i++)
			{
				dot[0][i]=0;
				dot[1][i]=0;
				dot[8][i]=0;
				dot[9][i]=0;
			}
			for(i=-1;i<2;i++)		//막대 출력
			{
				dot[p1.y][p1.x+i]=1;
				dot[p2.y][p2.x+i]=1;
			}
			draw();
		}
	}
	switch (textn){		//해당 텍스트 출력 (게임 키 설명)
		case 0:
			strcpy(mybuf3.mtext,"Welcome!        Press 6:next");
			break;
		case 1:
			strcpy(mybuf3.mtext,"Press 6:next    Press 4:prev");
			break;
		case 2:
			strcpy(mybuf3.mtext,"Press 4:prev    P1 2:up/down");
			break;
		case 3:
			strcpy(mybuf3.mtext,"P1 2:up/down    1:left 3:right");
			break;
		case 4:
			strcpy(mybuf3.mtext,"1:left 3:right  P2 8:up/down");
			break;
		case 5:
			strcpy(mybuf3.mtext,"P2 8:up/down    7:left 9:right");
			break;
		case 6:
			strcpy(mybuf3.mtext,"7:left 9:right  5:start game");
			break;
		case 7:
			strcpy(mybuf3.mtext,"5:start game    Enjoy the game!");
			break;
	}
}
int dec_to_otc(int num)	//10진수를 8진수로 바꾸는 함수
{
	int n1,n2,n3;
	num%=512;
	//n1=num/512;
	n1=(num%512)/64;
	n2=(num%64)/8;
	n3=num%8;
	return n1*100+n2*10+n3;
}
int otc_to_quad(int num)	//8진수를 4진수로 바꾸는 함수
{
	int n1,n2,n3;
	//n1=digit(num,4)*512;
	n1=digit(num,3)*64;
	n2=digit(num,2)*8;
	n3=digit(num,1);
	num=n1+n2+n3;	//8진수를 10진수로 바꾸고
	num%=64;
	//n1=num/64;
	n1=(num%64)/16;
	n2=(num%16)/4;
	n3=num%4;
	return n1*100+n2*10+n3;	//10진수를 4진수로 다시 바꿈
}
int quad_to_bi(int num)	//4진수를 2진수로 바꾸는 함수
{
	int n1,n2,n3;
	//n1=digit(num,4)*64;
	n1=digit(num,3)*16;
	n2=digit(num,2)*4;
	n3=digit(num,1);
	num=n1+n2+n3;	//4진수를 10진수로 바꾸고
	num%=8;
	//n1=num/8;
	n1=(num%8)/4;
	n2=(num%4)/2;
	n3=num%2;
	return n1*100+n2*10+n3;	//10진수를 2진수로 다시 바꿈
}
int bi_to_dec(int num)	//2진수를 10진수로 바꾸는 함수
{
	int n1,n2,n3;
	//n1=digit(num,4)*8;
	n1=digit(num,3)*4;
	n2=digit(num,2)*2;
	n3=digit(num,1);
	return n1+n2+n3;
}
int digit(int num,int dig)	//해당 자리수의 숫자를 리턴
{
	int result=pow(10,dig-1);
	return num/result%10;
}
int count_input(int num)	//입력받은 switch가 몇개인지 리턴
{
	int i,count=0;
	for(i=0;i<9;i++)
	{
		if(digit(num,i+1)==1)
			count++;
	}
	return count;
}
int find_switch(int num)	//눌린 스위치의 번호를 리턴
{
	int i;
	for(i=0;i<9;i++)
	{
		if(digit(num,i+1)==1)
			return 9-i;
	}
}
void init(){	//배열 초기화
	int i;
	for(i=0;i<9;i++)
		cnt[i]=0;
}
void reverse()	//그림을 반전 시킴
{
	int i,j;
	for(i=0;i<10;i++)
		for(j=0;j<7;j++)
			dot[i][j]=1-dot[i][j];
	dot[y][x]=1-prev;
}
void draw()	//그림을 그리는 함수
{
	int i,j;
	int result=0;
	for(j=0;j<10;j++)
	{
		for(i=0;i<7;i++)
		{
			result*=2;
			result+=dot[j][i];		//배열에 저장된 값을 십진수로 바꿔준다
		}
		mybuf3.dot[j]=result;
	}
}
void initialize(int mode){	//모드가 바뀔 때 출력 화면 초기화 하는 함수
	time_t timer;
	struct tm *t;
	int i;
	timer=time(NULL);
	t=localtime(&timer);
	fndnum=0;
	cmode=0;
	fcount=0;
	fmode=0;
	tmode=0;
	x=0;
	y=0;
	cursor=1;
	prev=0;
	p1.x=p2.x=3;
	p1.y=0;
	p2.y=9;
	p1.score=0;
	p2.score=0;
	ball1.y=4;
	ball1.z=3;
	ball1.dir=1;
	memset(dot,0,70);	//전역변수들 초기화
	if(mode==0)	//clock mode일 때의 초기화
	{
		mybuf3.msg=0;
		fndnum=t->tm_hour*100+t->tm_min;
		mybuf3.fnd=fndnum;
		mybuf3.led=128;		//
		memcpy(mybuf3.dot,fpga_number[0],10);
		strcpy(mybuf3.mtext,"");
		lednum=128;
	}
	else if(mode==1)	//counter mode일 때의 초기화
	{
		mybuf3.msg=0;
		mybuf3.fnd=0;
		mybuf3.led=64;
		memcpy(mybuf3.dot,fpga_number[0],10);
		strcpy(mybuf3.mtext,"");
		lednum=64;
	}
	else if(mode==2)	//textmode일 때의 초기화
	{
		mybuf3.msg=0;
		mybuf3.fnd=0;
		mybuf3.led=0;
		memcpy(mybuf3.dot,fpga_number[2],10);
		strcpy(mybuf3.mtext,"");
		//memset(mybuf3.mtext,0x00,33);
		lednum=0;
	}
	else if(mode==3)	//drawmode일 때의 초기화
	{
		mybuf3.msg=0;
		mybuf3.fnd=0;
		mybuf3.led=0;
		memcpy(mybuf3.dot,fpga_number[0],10);
		strcpy(mybuf3.mtext,"");
		lednum=0;
	}
	else		//additional mode일 때의 초기화
	{
		mybuf3.msg=0;
		mybuf3.fnd=0;
		mybuf3.led=0;
		start=0;
		end=0;
		textn=0;
		for(i=-1;i<2;i++)	//막대 출력 초기화
		{
			dot[p1.y][p1.x+i]=1;
			dot[p2.y][p2.x+i]=1;
		}
		draw();
		strcpy(mybuf3.mtext,"Welcome!        Press 6:next");	//문자열 출력
		lednum=0;
	}
}
