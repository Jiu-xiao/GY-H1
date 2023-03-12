#include "main.h"

#include "arm_math.h"
#include "WS281x.h"
#include "MsgThread.h"
#include "IMU.h"

uint16_t TxNum = 0;
uint16_t TxDir = 0;
uint16_t TxBuf= 0;
uint16_t TxTask = 0;
uint16_t InStackNum = 0;
uint8_t Test0 = 0;
uint8_t Test1 = 0;
uint8_t Test2 = 0;
uint8_t Test3 = 0;


cWS281x LED;

static void LEDThread(void* parameter);
static rt_thread_t LED_thread = RT_NULL;


static void Test1Thread(void* parameter);
static rt_thread_t Test1_thread = RT_NULL;
static void Test2Thread(void* parameter);
static rt_thread_t Test2_thread = RT_NULL;
static void Test3Thread(void* parameter);
static rt_thread_t Test3_thread = RT_NULL;
static void Test4Thread(void* parameter);
static rt_thread_t Test4_thread = RT_NULL;

extern void UARTThread(void* parameter);
extern rt_thread_t UART_thread;
extern rt_mutex_t UART0_TxMut;
extern rt_sem_t UART0_TxSem;	
extern rt_sem_t UART0_RxSem;

extern void USBDThread(void* parameter);
extern rt_thread_t USBD_thread;
extern rt_sem_t USBD_Sem;

extern void IMUThread(void* parameter);
extern void IMU2Thread(void* parameter);
extern rt_thread_t IMU_thread;
extern rt_thread_t IMUSlaver_thread;
extern rt_sem_t IMU_INT1Sem;	
extern rt_sem_t IMU_INT2Sem;

uint8_t dataXXX[128]={0x01,0x01,0x01,0x01};
uint8_t dataYYY[128]={0x02,0x02,0x02,0x02};
uint8_t dataZZZ[128]={0x03,0x03,0x03,0x03};
uint8_t dataUUU[128]={0x04,0x04,0x04,0x04};

uint8_t TxStatue[4];

int main(void)
{	
	LED_thread =                          					/* �߳̿��ƿ�ָ�� */
	rt_thread_create( 					"LED",             	/* �߳����� */
										LEDThread,   		/* �߳���ں��� */
										RT_NULL,            /* �߳���ں������� */
										512,                /* �߳�ջ��С */
										16,                  /* �̵߳����ȼ� */
										20);                /* �߳�ʱ��Ƭ */	
	
	Test1_thread =                          				/* �߳̿��ƿ�ָ�� */
	rt_thread_create( 					"Test1",            /* �߳����� */
										Test1Thread,   		/* �߳���ں��� */
										RT_NULL,            /* �߳���ں������� */
										512,                /* �߳�ջ��С */
										3,                  /* �̵߳����ȼ� */
										5);                /* �߳�ʱ��Ƭ */
	Test2_thread =                          				/* �߳̿��ƿ�ָ�� */
	rt_thread_create( 					"Test2",            /* �߳����� */
										Test2Thread,   		/* �߳���ں��� */
										RT_NULL,            /* �߳���ں������� */
										512,                /* �߳�ջ��С */
										3,                  /* �̵߳����ȼ� */
										5);                /* �߳�ʱ��Ƭ */
	Test3_thread =                          				/* �߳̿��ƿ�ָ�� */
	rt_thread_create( 					"Test3",            /* �߳����� */
										Test3Thread,   		/* �߳���ں��� */
										RT_NULL,            /* �߳���ں������� */
										128,                /* �߳�ջ��С */
										4,                  /* �̵߳����ȼ� */
										5);                /* �߳�ʱ��Ƭ */
	Test4_thread =                          				/* �߳̿��ƿ�ָ�� */
	rt_thread_create( 					"Test4",            /* �߳����� */
										Test4Thread,   		/* �߳���ں��� */
										RT_NULL,            /* �߳���ں������� */
										128,                /* �߳�ջ��С */
										5,                  /* �̵߳����ȼ� */
										5);                /* �߳�ʱ��Ƭ */
	
	UART_thread =                          					/* �߳̿��ƿ�ָ�� */
	rt_thread_create( 					"UART",             /* �߳����� */
										UARTThread,   		/* �߳���ں��� */
										RT_NULL,            /* �߳���ں������� */
										512,                /* �߳�ջ��С */
										2,                  /* �̵߳����ȼ� */
										1);                 /* �߳�ʱ��Ƭ */
	
	UART0_TxMut =
	rt_mutex_create(					"UART0_TxMut",		/* ������������ */
										RT_IPC_FLAG_FIFO);	/* �������ı�־λ */

	UART0_TxSem	=
	rt_sem_create(						"UART0_TxSem",		/* �ź��������� */
										0,					/* ��ʼ����ֵ */
										RT_IPC_FLAG_FIFO);	/* �ź����ı�־λ */
	
	UART0_RxSem	=
	rt_sem_create(						"UART0_RxSem",		/* �ź��������� */
										0,					/* ��ʼ����ֵ */
										RT_IPC_FLAG_FIFO);	/* �ź����ı�־λ */
										
	USBD_thread =                          					/* �߳̿��ƿ�ָ�� */
	rt_thread_create( 					"USBD",             /* �߳����� */
										USBDThread,   		/* �߳���ں��� */
										RT_NULL,            /* �߳���ں������� */
										512,               /* �߳�ջ��С */
										2,                  /* �̵߳����ȼ� */
										1);                 /* �߳�ʱ��Ƭ */

	USBD_Sem	=
	rt_sem_create(						"USBD_Sem",			/* �ź��������� */
										0,					/* ��ʼ����ֵ */
										RT_IPC_FLAG_FIFO);	/* �ź����ı�־λ */

	IMU_thread =                          					/* �߳̿��ƿ�ָ�� */
	rt_thread_create( 					"IMU",              /* �߳����� */
										IMUThread,   		/* �߳���ں��� */
										RT_NULL,            /* �߳���ں������� */
										512,                /* �߳�ջ��С */
										1,                  /* �̵߳����ȼ� */
										10);                 /* �߳�ʱ��Ƭ */	

	IMUSlaver_thread =                          			/* �߳̿��ƿ�ָ�� */
	rt_thread_create( 					"IMUSlaver",        /* �߳����� */
										IMU2Thread,   		/* �߳���ں��� */
										RT_NULL,            /* �߳���ں������� */
										256,                /* �߳�ջ��С */
										1,                  /* �̵߳����ȼ� */
										1);                 /* �߳�ʱ��Ƭ */
										
	IMU_INT1Sem	=
	rt_sem_create(						"IMU_INT1Sem",		/* �ź��������� */
										0,					/* ��ʼ����ֵ */
										RT_IPC_FLAG_FIFO);	/* �ź����ı�־λ */
										
	IMU_INT2Sem	=
	rt_sem_create(						"IMU_INT2Sem",		/* �ź��������� */
										0,					/* ��ʼ����ֵ */
										RT_IPC_FLAG_FIFO);	/* �ź����ı�־λ */										
	/* �����̣߳��������� */
	rt_thread_startup(UART_thread);
	rt_thread_startup(USBD_thread);
	#ifdef qwDbug									
	rt_kprintf("\n\nUART_thread  = %d\n",UART_thread);rt_thread_delay(2);	
	rt_kprintf("LED_thread   = %d\n",LED_thread);rt_thread_delay(2);
	rt_kprintf("Test1_thread = %d\n",Test1_thread);rt_thread_delay(2);
	rt_kprintf("Test2_thread = %d\n",Test2_thread);rt_thread_delay(2);	
	rt_kprintf("Test3_thread = %d\n",Test3_thread);rt_thread_delay(2);
	rt_kprintf("Test4_thread = %d\n",Test4_thread);rt_thread_delay(2);				
	#endif
	rt_thread_startup(LED_thread);					
	rt_thread_startup(Test1_thread);
	rt_thread_startup(IMU_thread);
	rt_thread_startup(IMUSlaver_thread);
	rt_thread_startup(Test2_thread);
//	rt_thread_startup(Test3_thread);
//	rt_thread_startup(Test4_thread);
	return 0;
}


static void LEDThread(void* parameter)
{	rt_thread_delay(100);
	LEDColor_t Color={0};
	uint8_t Color_buf[10]={0};
	rt_tick_t ticks;

	float VFie  = PI/127;
	float VTheta = PI/163;
	
	float Theta = PI/7;
	float Fie = 0;
	uint16_t CubeHalfHeight = 40;
	
	LED.cSPI::SPI_Init(SPI2,GPIOB,GPIO_PIN_6,DMA1,DMA_CH1);
	LED.Init(Color_buf);
	LED.LED_UpdateDMA(&Color,1);
	for(;;)
	{	
		ticks = rt_tick_get();
	
		Theta += VTheta;
		Fie	  += VFie;
		
		if(Theta>2*PI){Theta-=(2*PI);}
		else if(Theta<0){Theta+=(2*PI);}
		
		if(Fie>2*PI){Fie-=(2*PI);}
		else if(Fie<0){Fie+=(2*PI);}
		
		Color.GRB[0] = CubeHalfHeight*(arm_sin_f32(Fie)+1);
		Color.GRB[1] = CubeHalfHeight*(arm_sin_f32(Theta)+1);
		Color.GRB[2] = CubeHalfHeight*(arm_cos_f32(Theta)+1);
		
		LED.LED_UpdateDMA(&Color,1); 
		rt_thread_delay_until(&ticks,10);
	}
}

static void Test1Thread(void* parameter)
{
	rt_thread_delay(1000);

	for(;;)
	{	

		while(gpio_input_bit_get(GPIOB,GPIO_PIN_4))rt_thread_delay(10);
		#ifdef qwDbug
		list_thread();
		#endif
		rt_thread_delay(1000);
		
	}
}
static void Test2Thread(void* parameter)
{
	rt_thread_delay(2000);

	for(;;)
	{	
		MSG_TX->Printf("GYRO:  %d %d %d\n",IMU->Gyro[0],IMU->Gyro[1],IMU->Gyro[2]);rt_thread_delay(1);
		MSG_TX->Printf("Accel: %d %d %d\n",IMU->Accel[0],IMU->Accel[1],IMU->Accel[2]);rt_thread_delay(1);
		MSG_TX->Printf("Tem: %f\n\n",IMU->Temperature);
		rt_thread_delay(490);
	}
}
static void Test3Thread(void* parameter)
{
	rt_thread_delay(1000);

	for(;;)
	{	
		MSG_TX->MSGTx(0xAA,dataZZZ,12);
		rt_thread_delay(10);
	}
}
static void Test4Thread(void* parameter)
{
	rt_thread_delay(1000);

	for(;;)
	{	
		MSG_TX->MSGTx(dataUUU,12);
		rt_thread_delay(10);
	}
}

void DMA1_Channel1_IRQHandler(void)
{
	LED.cSPI::IRQ_Tx();
}