#include "MsgThread.h"

extern uint16_t TxNum;
extern uint16_t InStackNum;
/*���ڿ�����*/
cUART *myUART0;
/*���򴮿ڷ��Ϳ���*/
cMSG MSG_TX;
/*���̿���ָ��*/
rt_thread_t UART_thread = RT_NULL;
/*���Ϳ��п����ź���*/
rt_sem_t UART0_TxSem = RT_NULL;
/*������Ϣ����ָ��*/
struct rt_messagequeue UART0_TxMSG;

/*
	���ڷ��ͽ���
	ʹ������Ϣ���еķ�ʽ����Ӳ����Դ��֧�ַ��ͽ�����Ϣ
	����Ӳ����Դ�ĳ�ʼ���ڴ˽��У����ڽ�����Ӧ��Ҫ����Ӳ����Դ��ʼ��ִ��
*/
void UARTThread(void* parameter)
{
	/*	CFGBuf
		����������Ϣ������ ����Ϊ3 
		Pack[0]���Ϊ��ʾ�Ƿ������ͷ
		Pack[0] 0~7 �� Pack[1] 0~8�������ݳ���
		Pack[1]���а�ͷʱ��ʾ��ͷ���ް�ͷʱ��Ϣ������
	*/
	static uint8_t RecBuf[UART_MSG_CFG_LEN+UART_MSG_MAX_LEN];
	uint8_t IsHead = 0;//�Ƿ���Ҫ��Ӱ�ͷ
	uint16_t TxLen = 0;//���ݲ��ֳ���

	/*��ʼ��Ӳ����Դ*/
	myUART0 = new cUART;
	myUART0->UART_Init(USART0,DMA0,DMA_CH4,DMA0,DMA_CH3);
	for(;;)
	{
		/*UARTӲ������*/
		rt_sem_take(UART0_TxSem,3);
		/*��ȡ�������ݰ�*/
		rt_mq_recv(&UART0_TxMSG,RecBuf,UART_MSG_CFG_LEN+UART_MSG_MAX_LEN,RT_WAITING_FOREVER);
		/*���㷢�Ͳ��ֳ���*/
		TxLen = RecBuf[0]<<8 | RecBuf[1];
		/*�ж��Ƿ���Ҫ��Ӱ�ͷ*/
		IsHead = RecBuf[0]&0x80;
		if(IsHead)
		{
			/*����Ӳ������*/
			myUART0->Transmit_DMA(RecBuf+2,TxLen);
		}
		else
		{
			myUART0->Transmit_DMA(RecBuf+3,TxLen);
		}
		InStackNum --;
		TxNum++;
	} 
}

void DMA0_Channel3_IRQHandler(void)
{
	myUART0->Transmit_IRQ();
	rt_sem_release(UART0_TxSem);
}
void rt_hw_console_output(const char *str)
{
	if(UART_thread==RT_NULL){
    /* �����ٽ�� */
	rt_enter_critical();
	while(*str != '\0')
	{
		if( *str=='\n' )
		{	
			while (!usart_flag_get(USART0, USART_FLAG_TBE)){}
			usart_data_transmit(USART0,'\r');
			while (!usart_flag_get(USART0, USART_FLAG_TBE)){}
		}
		
		while (!usart_flag_get(USART0, USART_FLAG_TBE)){}
		usart_data_transmit(USART0,*str++);
		while (!usart_flag_get(USART0, USART_FLAG_TBE)){}
	}
	/* �˳��ٽ�� */
	rt_exit_critical();
	}
	else
	{
		uint16_t i=0;
		while(*(str+i) != '\0'){i++;}
		MSG_TX.MSGTx((uint8_t *)str,i);
	}
}
/*
	������Ϣ���ͽ���,�ް�ͷ
	������FIFO�߼���Ӧ
	Len:ע�� ���Ϊ Min(32768,UART_MSG_MAX_LEN)
*/
uint8_t cMSG::MSGTx(uint8_t *pdata, uint16_t Length)
{
	if(pdata==0){return 1;}
	if(Length>UART_MSG_MAX_LEN){return 1;}
	
	
	
	uint8_t *buf = (uint8_t *)rt_malloc(UART_MSG_CFG_LEN+UART_MSG_MAX_LEN);
	
	buf[0] = Length>>8;
	buf[1] = Length&0xFF;
	
	memcpy(buf+3,pdata,Length);
	if(rt_mq_send(&UART0_TxMSG,buf,UART_MSG_CFG_LEN+UART_MSG_MAX_LEN)==-RT_EFULL)
	{rt_free(buf);return 1;}
	
	rt_free(buf);
	InStackNum++;
	return 0;
}

/*
	������Ϣ���ͽ���,�а�ͷ
	������FIFO�߼���Ӧ
	Len:ע�� ���Ϊ Min(32768,UART_MSG_MAX_LEN)
*/
uint8_t cMSG::MSGTx(uint8_t Head, uint8_t *pdata, uint16_t Length)
{
	if(pdata==0){return 1;}
	if(Length>UART_MSG_MAX_LEN){return 1;}
	
	uint8_t *buf = (uint8_t *)rt_malloc(UART_MSG_CFG_LEN+UART_MSG_MAX_LEN);
	
	uint8_t CFGBuf[3]={0};
	CFGBuf[0] = Length>>8;
	CFGBuf[0] |=  0x80;
	CFGBuf[1] = Length&0xFF;
	CFGBuf[2] = Head;
	
	memcpy(buf+3,pdata,Length);
	if(rt_mq_send(&UART0_TxMSG,buf,UART_MSG_CFG_LEN+UART_MSG_MAX_LEN)==-RT_EFULL)
	{rt_free(buf);return 1;}
	
	rt_free(buf);
	InStackNum++;
	return 0;
}

/*
	������Ϣ���ͽ���,�ް�ͷ
	��Ϣ���������͵����Ͷ���λ��
	Len:ע�� ���Ϊ Min(32768,UART_MSG_MAX_LEN)
*/
uint8_t cMSG::MSGUrgentTx(uint8_t *pdata, uint16_t Length)
{
	if(pdata==0){return 1;}
	if(Length>UART_MSG_MAX_LEN){return 1;}
	
	uint8_t *buf = (uint8_t *)rt_malloc(UART_MSG_CFG_LEN+UART_MSG_MAX_LEN);
	
	buf[0] = Length>>8;
	buf[1] = Length&0xFF;
	
	memcpy(buf+3,pdata,Length);
	if(rt_mq_urgent(&UART0_TxMSG,buf,UART_MSG_CFG_LEN+UART_MSG_MAX_LEN)==-RT_EFULL)
	{rt_free(buf);return 1;}
	
	rt_free(buf);
	InStackNum++;
	return 0;
}

/*
	������Ϣ���ͽ���,�а�ͷ
	��Ϣ���������͵����Ͷ���λ��
	Len:ע�� ���Ϊ Min(32768,UART_MSG_MAX_LEN)
*/
uint8_t cMSG::MSGUrgentTx(uint8_t Head, uint8_t *pdata, uint16_t Length)
{
	if(pdata==0){return 1;}
	if(Length>UART_MSG_MAX_LEN){return 1;}
	
	uint8_t *buf = (uint8_t *)rt_malloc(UART_MSG_CFG_LEN+UART_MSG_MAX_LEN);
	
	uint8_t CFGBuf[3]={0};
	CFGBuf[0] = Length>>8;
	CFGBuf[0] |=  0x80;
	CFGBuf[1] = Length&0xFF;
	CFGBuf[2] = Head;
	
	memcpy(buf+3,pdata,Length);
	if(rt_mq_urgent(&UART0_TxMSG,buf,UART_MSG_CFG_LEN+UART_MSG_MAX_LEN)==-RT_EFULL)
	{rt_free(buf);return 1;}
	
	rt_free(buf);
	InStackNum++;
	return 0;
}