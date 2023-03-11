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

/*
	���ڿ��ƽ���
	���ƴ�����Ϣ�Ľ���
	����Ӳ����Դ�ĳ�ʼ���ڴ˽��У����ڽ�����Ӧ��Ҫ����Ӳ����Դ��ʼ��ִ��
*/
void UARTThread(void* parameter)
{
	uint8_t IsHead = 0;//�Ƿ���Ҫ��Ӱ�ͷ
	uint16_t TxLen = 0;//���ݲ��ֳ���

	MSG_TX = new cMSG;
	MSG_TX->UART_Init(USART0,DMA0,DMA_CH4,DMA0,DMA_CH3);
	for(;;)
	{
		/*�ȴ��������*/
		rt_sem_take(UART0_RxSem,RT_WAITING_FOREVER);
	}
}

void DMA0_Channel3_IRQHandler(void)
{
	if(MSG_TX->Transmit_IRQ()){
	rt_sem_release(UART0_TxSem);
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
	/*ֱ��ʹ��DMA����*/
	rt_mutex_take(UART0_TxMut,RT_WAITING_NO);
	this->Transmit_DMA(pdata,Length);
	rt_sem_take(UART0_TxSem,RT_WAITING_FOREVER);
	rt_mutex_release(UART0_TxMut);
	return 0;
}
/*
	�����溯���Ļ����ϣ����һ�����ݰ�ͷ
*/
uint8_t cMSG::MSGTx(uint8_t Head, uint8_t *pdata, uint16_t Length)
{
	if(pdata==0){return 1;}
	/*ֱ��ʹ��DMA����*/
	rt_mutex_take(UART0_TxMut,RT_WAITING_NO);
	usart_data_transmit(USART0,Head);
	while (!usart_flag_get(USART0, USART_FLAG_TBE)){}
	this->Transmit_DMA(pdata,Length);
	rt_sem_take(UART0_TxSem,RT_WAITING_FOREVER);
	rt_mutex_release(UART0_TxMut);
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