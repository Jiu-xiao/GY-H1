#include "main.h"

#include "arm_math.h"
#include "WS281x.h"
#include "MsgThread.h"

uint16_t TxNum = 0;
uint16_t InStackNum = 0;

cWS281x LED;



static rt_thread_t LED_thread = RT_NULL;
static void LEDThread(void* parameter);

static rt_thread_t Test1_thread = RT_NULL;
static void Test1Thread(void* parameter);

extern rt_thread_t UART_thread;
extern void UARTThread(void* parameter);
	
extern rt_sem_t UART0_TxSem;	

extern struct rt_messagequeue UART0_TxMSG;
static rt_uint8_t UART0_TxMSG_POOL[(UART_MSG_MAX_LEN+UART_MSG_CFG_LEN)*16];

static uint8_t dataXXX[128]={0x01,0x02,0x03,0x01};


int main(void)
{	
	LED_thread =                          					/* �߳̿��ƿ�ָ�� */
	rt_thread_create( 					"LED",             	/* �߳����� */
										LEDThread,   		/* �߳���ں��� */
										RT_NULL,            /* �߳���ں������� */
										256,                /* �߳�ջ��С */
										3,                  /* �̵߳����ȼ� */
										20);                /* �߳�ʱ��Ƭ */
	
	Test1_thread =                          				/* �߳̿��ƿ�ָ�� */
	rt_thread_create( 					"Test1",            /* �߳����� */
										Test1Thread,   		/* �߳���ں��� */
										RT_NULL,            /* �߳���ں������� */
										256,                /* �߳�ջ��С */
										5,                  /* �̵߳����ȼ� */
										20);                /* �߳�ʱ��Ƭ */

	UART_thread =                          					/* �߳̿��ƿ�ָ�� */
	rt_thread_create( 					"UART",             /* �߳����� */
										UARTThread,   		/* �߳���ں��� */
										RT_NULL,            /* �߳���ں������� */
										512,                /* �߳�ջ��С */
										1,                  /* �̵߳����ȼ� */
										20);                /* �߳�ʱ��Ƭ */
	
	UART0_TxSem	=
	rt_sem_create(						"UART0_TxSem",		/*�ź���������*/
										1,					/*��ʼ����ֵ*/
										RT_IPC_FLAG_FIFO);	/*�ź����ı�־λ*/
	
	rt_mq_init(
										&UART0_TxMSG,						/*��Ϣ����ָ��*/
										"UART0_TxMSG",						/*��Ϣ����*/
										&UART0_TxMSG_POOL[0],				/*�ڴ��*/
										UART_MSG_MAX_LEN+UART_MSG_CFG_LEN,	/*ÿ����Ϣ�ĳ���*/
										sizeof(UART0_TxMSG_POOL),			/*�ڴ�ش�С*/
										RT_IPC_FLAG_FIFO);					/*�����ȳ�*/
										


										
	/* �����̣߳��������� */
	rt_thread_startup(LED_thread);
	rt_thread_startup(UART_thread);
	rt_thread_startup(Test1_thread);
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
	rt_thread_delay(100);

	for(;;)
	{	
//		rt_kprintf("12345\n");
//		MSG_TX.MSGTx(dataXXX,40);
//		MSG_TX.MSGTx(dataXXX,128);
//		MSG_TX.MSGTx(dataXXX,128);
		rt_thread_delay(10);
	}
}

void DMA1_Channel1_IRQHandler(void)
{
	LED.cSPI::IRQ_Tx();
}