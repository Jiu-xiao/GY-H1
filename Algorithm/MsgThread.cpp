#include "MsgThread.h"

/*���򴮿ڷ��Ϳ���*/
cMSG  *MSG_TX;
/*���̿���ָ��*/
rt_thread_t UART_thread = RT_NULL;
/*������ɿ����ź���*/
rt_sem_t UART0_TxSem = RT_NULL;
/*������ɿ����ź���*/
rt_sem_t UART0_RxSem = RT_NULL;
/*���Ϳ��ƻ�����*/
rt_mutex_t UART0_TxMut = RT_NULL;
/*������Ϣ����ָ��*/
rt_messagequeue UART0_TxMsq;

/*
	���ڿ��ƽ���
	���ƴ�����Ϣ�Ľ���
	����Ӳ����Դ�ĳ�ʼ���ڴ˽��У����ڽ�����Ӧ��Ҫ����Ӳ����Դ��ʼ��ִ��
*/
void UARTThread(void* parameter)
{
	uint8_t IsHead = 0;//�Ƿ���Ҫ��Ӱ�ͷ
	uint16_t TxLen = 0;//���ݲ��ֳ���
	static uint8_t RecBuf[UART_MSG_CFG_LEN+UART_MSG_MAX_LEN];
	
	MSG_TX = new cMSG;
	MSG_TX->UART_Init(USART0,DMA0,DMA_CH4,DMA0,DMA_CH3);
	for(;;)
	{
		/*��ȡ�������ݰ�*/
		rt_mq_recv(&UART0_TxMsq,RecBuf,UART_MSG_CFG_LEN+UART_MSG_MAX_LEN,RT_WAITING_FOREVER);
		/*���󻥳���*/
		rt_mutex_take(UART0_TxMut,RT_WAITING_FOREVER);
		/*���㷢�Ͳ��ֳ���*/
		TxLen = RecBuf[0]<<8 | RecBuf[1];
		/*�ж��Ƿ���Ҫ��Ӱ�ͷ*/
		IsHead = RecBuf[0]&0x80;
		/*����Ӳ������*/
		if(IsHead)
		{MSG_TX->Transmit_DMA(RecBuf+2,TxLen);}
		else
		{MSG_TX->Transmit_DMA(RecBuf+3,TxLen);}
		/*������������-1*/
		MSG_TX->TxBufLen--;
		/*���Delay*/
		rt_thread_delay(1);
		/*�ȴ��������*/
		rt_sem_take(UART0_TxSem,RT_WAITING_FOREVER);
		/*�ͷŻ�����*/
		rt_mutex_release(UART0_TxMut);
	}
}

void DMA0_Channel3_IRQHandler(void)
{
	if(MSG_TX->Transmit_IRQ()){
	rt_sem_release(UART0_TxSem);
	if(!MSG_TX->TxBufLen)
	{MSG_TX->TxDirOK = 1;}
	}
}

/*
	���ݷ���״̬����ѹ�뻺�����ֱ�ӷ���
	ֱ�ӷ���ʱ:
	����ʹ�û���������������Դ
	ʹ���ź����жϷ����Ƿ����
	��������ֱ�ӷ�����ɺ��������
	ѹ�뻺��ʱ:
	ʹ����Ϣ������Ϊ������
*/
uint8_t cMSG::MSGTx(uint8_t *pdata, uint16_t Length)
{
	if(pdata==0){return 1;}
	
//	rt_enter_critical();
//	uint8_t IsDirOk = this->TxDirOK;
//	rt_exit_critical();
//	
//	if(IsDirOk == 1)//����ֱ�ӷ���
//	{
//		/*ֱ��ʹ��DMA����*/
//		rt_mutex_take(UART0_TxMut,RT_WAITING_NO);
//		this->TxDirOK = 0;
//		this->Transmit_DMA(pdata,Length);
//		rt_sem_take(UART0_TxSem,RT_WAITING_FOREVER);
//		rt_mutex_release(UART0_TxMut);
//		rt_thread_delay(1);
//	}
//	else//ѹ�뷢�ͻ�����
//	{
		uint8_t *buf = (uint8_t *)rt_malloc(UART_MSG_CFG_LEN+UART_MSG_MAX_LEN);
		
		buf[0] = Length>>8;
		buf[1] = Length&0xFF;
		
		rt_memcpy(buf+3,pdata,Length);

		if(rt_mq_send(&UART0_TxMsq,buf,UART_MSG_CFG_LEN+UART_MSG_MAX_LEN)!=-RT_EOK)
		{
			/*��������*/
			rt_free(buf);
			return 1;
		}
		MSG_TX->TxBufLen++;
		rt_free(buf);
//	}
	return 0;
}


void rt_hw_console_output(const char *str)
{
	#ifndef qwDbug
	if(UART_thread==RT_NULL){
	#endif
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
	#ifndef qwDbug
	}
	else
	{
		uint16_t i=0;
		while(*(str+i) != '\0'){i++;}
		MSG_TX->MSGTx((uint8_t *)str,i);
	}
	#endif
}