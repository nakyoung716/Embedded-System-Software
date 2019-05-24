#include "total.h"
//unsigned char blank[10]={0,0,0,0,0,0,0,0,0,0};
unsigned char quit=0;
void user_signal1(int sig)
{
	quit=1;
}
key_t key_id1, key_id2;
int i;
struct msgbuf mybuf1,mybuf2;
int in_main(){
	pthread_t p_thread[2];
	int thr_id;

	key_id1 = msgget((key_t)1234, IPC_CREAT|0666);	//메세지 생성
	if (key_id1 == -1)
	{
		perror("msgget error 2: ");
		exit(0);
	}
	key_id2 = msgget((key_t)5678, IPC_CREAT|0666);
	if (key_id2 == -1)
	{
		perror("msgget error 3: ");
		exit(0);
	}
	
	memset(mybuf1.mtext, 0x00, 33); 
	strcpy(mybuf1.mtext,"");
	mybuf1.msg = 0;
	mybuf1.fnd = 0;
	mybuf1.led = 0;
	memset(mybuf1.dot,0,10);
	
	memset(mybuf2.mtext, 0x00, 33); 
	strcpy(mybuf2.mtext,"");
	mybuf2.msg = 0;
	mybuf2.fnd = 0;
	mybuf2.led = 0;
	memset(mybuf2.dot,0,10);		//메세지 초기화

	thr_id=pthread_create(&p_thread[0],NULL,input_key,NULL);	//key값을 받아 넘기는 함수를 쓰레드로 돌림
	thr_id=pthread_create(&p_thread[1],NULL,input_switch,NULL);		//switch값을 받아서 넘기는 함수를 쓰레드로 돌림
	pthread_join(p_thread[0],(void **)NULL);
	pthread_join(p_thread[1],(void **)NULL);
	return 0;
}
void* input_key() //key값을 입력받아 main으로 보내는 함수
{
	struct input_event ev[BUFF_SIZE];
	int fd, rd, value, size = sizeof (struct input_event);

	char* device = "/dev/input/event0";
	if((fd = open (device, O_RDONLY)) == -1) {		//device open에러 처리
		printf ("%s is not a vaild device.n", device);
	}
	while (1){
		if ((rd = read (fd, ev, size * BUFF_SIZE)) < size)
		{
			printf("read()");  
			return (0);     
		}

		value = ev[0].value;

		if (value != ' ' && ev[1].value == 1 && ev[1].type == 1){ // Only read the key press event
			printf ("code%d\n", (ev[1].code));
		}
		if( value == KEY_RELEASE ) {
			printf ("key release\n");
		} else 				if( value == KEY_PRESS ) {
			printf ("key press\n");

			mybuf1.msgtype=3;
			mybuf1.msg=ev[0].code;
			if (msgsnd(key_id1, (void *)&mybuf1, sizeof(struct msgbuf), IPC_NOWAIT) == -1)		//input받은 key값을 main에 전달
			{
				perror("msgsnd error1 : ");
				exit(0);
			}
			printf("send key%d\n", ev[0].code);
			sleep(1);
		}
	}

}
void* input_switch() //switch값을 입력받아 main에 보내는 함수
{
	//switch
	int i;
	int dev;
	int buff_size;
	int result;
	int prev_value=0;

	unsigned char push_sw_buff[MAX_BUTTON];

	dev = open("/dev/fpga_push_switch", O_RDWR);	//device open

	if (dev<0){			//에러처리
		printf("Device Open Error\n");
		close(dev);
		return ;
	}

	(void)signal(SIGINT, user_signal1);		//시그널

	buff_size=sizeof(push_sw_buff);
	printf("Press <ctrl+c> to quit. \n");
	while(!quit){		//컨트롤c 누를때까지
		usleep(400000);
		read(dev, &push_sw_buff, buff_size);	//입력을 받아
		mybuf2.msgtype=4;
		result=0;
		for(i=0;i<MAX_BUTTON;i++) {	//눌린 스위치에 대한 정보를 저장
			result*=10;
			if(push_sw_buff[i]==1)
			{
				result+=1;
			}
		}
		mybuf2.msg=result;
		if(result!=0&&result!=prev_value)		//스위치를 쭉 누를때의 예외처리
		{
			if (msgsnd(key_id2, (void *)&mybuf2, sizeof(struct msgbuf), IPC_NOWAIT) == -1)	//switch값을 main에 전달
			{
				perror("msgsnd error2 : ");
				exit(0);
			}
			printf("send swit%d\n",result);
		}
		prev_value=result;
	}
	close(dev);
}
