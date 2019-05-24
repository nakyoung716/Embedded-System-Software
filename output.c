#include"total.h"
struct msgbuf mybuf;
void* rcv_msg();
key_t key_id;
int out_main(){	//output process
	int thr_id;
	pthread_t p_thread[1];
	clockmode(0);	
	thr_id=pthread_create(&p_thread[0],NULL,rcv_msg,NULL);	//main에서 받는 메세지를 받는 쓰레드를 생성
	pthread_join(p_thread[0],(void **)NULL);
	return 0;
}
void* rcv_msg(){	//main에서 넘긴 메세지를 받는 함수
	int msgtype;
	msgtype = 3;

	key_id = msgget(4321, IPC_CREAT|0666);
	if (key_id < 0)
	{
		perror("msgget error : ");
		exit(0);
	}
	while(1)	//무한으로 돌며 메세지를 전달받았는지 체크
	{
		if (msgrcv( key_id, (void *)&mybuf, sizeof(struct msgbuf), msgtype, 0) == -1)	//메세지 받을 때 오류가 생긴 경우
		{
			perror("msgrcv4 error : ");
			exit(0);    
		}
		printf("%d\n", mybuf.fnd);
		output_led(mybuf.led);	//led 출력 함수 호출
		output_fnd(mybuf.fnd);	//fnd 출력 함수 호출
		output_dot(mybuf.dot);	//dot 출력 함수 호출
		output_lcd(mybuf.mtext);//lcd 출력 함수 호출
	}
	//exit(0);
	return 0;
}
int output_led(int num)	//led 출력 함수
{
	int fd,i;
	unsigned long *fpga_addr = 0;
	unsigned char *led_addr =0;
	unsigned char data;
	data=num;
	if( (data<0) || (data>255) ){	//범위 벗어남 오류
		printf("Invalid range!\n");
		exit(1);
	}

	fd = open("/dev/mem", O_RDWR | O_SYNC); //memory device open
	if (fd < 0) {  //open fail check
		perror("/dev/mem open error");
		exit(1);
	}
	fpga_addr = (unsigned long *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, FPGA_BASE_ADDRESS);
	if (fpga_addr == MAP_FAILED) //mapping fail check
	{
		printf("mmap error!\n");
		close(fd);
		exit(1);
	}
	led_addr=(unsigned char*)((void*)fpga_addr+LED_ADDR);

	*led_addr=data;  

	//sleep(1);

	data=0;
	data=*led_addr; //read led
	printf("Current LED VALUE : %d\n",data);

	munmap(led_addr, 4096); 
	close(fd);
	return 0;
}

int output_fnd(int num)	//fnd 출력 함수
{
	int dev;
	unsigned char data[4];
	unsigned char retval;
	int i;
	int str_size;
	memset(data,0,sizeof(data));
	data[0]=num/1000;
	data[1]=num%1000/100;
	data[2]=num%100/10;
	data[3]=num%10;			//파라미터로 받은 숫자를 저장
	dev = open(FND_DEVICE, O_RDWR);	//file open
	if (dev<0) {
		printf("Device open error : %s\n",FND_DEVICE);
		exit(1);
	}
	retval=write(dev,&data,4);	//fnd 출력
	if(retval<0) {
		printf("Write Error!\n");
		return -1;
	}
	memset(data,0,sizeof(data));
	//sleep(1);
	/*retval=read(dev,&data,4);	
	if(retval<0) {
		printf("Read Error!\n");
		return -1;
	}*/
	printf("Current FND Value : ");
	for(i=0;i<str_size;i++)
		printf("%d",data[i]);
	printf("\n");
	close(dev);		//file close
	return(0);
}


int output_dot(unsigned char* img)	//dot 출력 함수
{
	int dev;

	dev = open(FPGA_DOT_DEVICE, O_WRONLY);	//file open
	if (dev<0) {
		printf("Device open error : %s\n",FPGA_DOT_DEVICE);
		exit(1);
	}

	write(dev,img,10);		//dot값 출력

	close(dev);		//file close
	return 0;
}

int output_lcd(char* text)	//lcd 출력 함수
{
	int dev;
	unsigned char string[32];

	memset(string,0,sizeof(string));
	dev = open(FPGA_TEXT_LCD_DEVICE, O_WRONLY);	//file open
	if(dev<0){
		printf("Device open error : %s\n",FPGA_TEXT_LCD_DEVICE);
		return -1;
	}
	strncpy(string,text,32);	//문자열 카피
	memset(string+strlen(text),' ',MAX_BUFF-strlen(text));	//문자열이 끝나면 공백입력
	write(dev,string,MAX_BUFF);	//문자열 출력

	close(dev);	//file close

	return(0);
}	
