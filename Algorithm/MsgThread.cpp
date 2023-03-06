#include "MsgThread.h"

/*���ڿ�����*/
cUART *myUART0;
/*���򴮿ڷ��Ϳ���*/
cMSG MSG_UART0;
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
	uint8_t CFGBuf[UART_MSG_CFG_LEN]={0};
	uint8_t IsHead = 0;//�Ƿ���Ҫ��Ӱ�ͷ
	
	/*	TxBuf
		����������Ϣ������ ����Ϊ���ݲ��ֳ���+1
	*/
	uint8_t TxBuf[UART_MSG_MAX_LEN+1]={0};
	uint16_t TxLen = 0;//���ݲ��ֳ���

	/*��ʼ��Ӳ����Դ*/
	myUART0 = new cUART;
	myUART0->UART_Init(USART0,DMA0,DMA_CH4,DMA0,DMA_CH3);
	for(;;)
	{
		/*UARTӲ������*/
		rt_sem_take(UART0_TxSem,RT_WAITING_FOREVER);
		/*��ȡ����������Ϣ*/
		rt_mq_recv(&UART0_TxMSG,CFGBuf,UART_MSG_CFG_LEN,RT_WAITING_FOREVER);
		/*���㷢�Ͳ��ֳ���*/
		TxLen = CFGBuf[0]<<8 | CFGBuf[1];
		/*�ж��Ƿ���Ҫ��Ӱ�ͷ*/
		IsHead = CFGBuf[0]&0x80;
		if(IsHead){TxBuf[0]=CFGBuf[2];}
		/*��ȡ���ݲ���,�������ް�ͷ������ƫ��*/
		if(rt_mq_recv(&UART0_TxMSG,TxBuf+IsHead,TxLen,RT_WAITING_FOREVER)==RT_EOK)
		{
			/*����Ӳ������*/
			myUART0->Transmit_DMA(TxBuf,TxLen+IsHead);
		}
		else/*�����ʱ��˵����Ϣ�����Ѿ�ʧȥͬ�������Իָ�����*/
		{
			rt_mq_delete(&UART0_TxMSG);//need to write
		}
	} 
}

void DMA0_Channel3_IRQHandler(void)
{
	if(myUART0->Transmit_IRQ())
	{
		rt_sem_release(UART0_TxSem);
	}

}
/*
	������Ϣ���ͽ���,�ް�ͷ
	������FIFO�߼���Ӧ
	Len:ע�� ���Ϊ Min(32768,UART_MSG_MAX_LEN)
*/
uint8_t cMSG::MSGTx(uint8_t *pdata, uint16_t Length)
{
	if(pdata==NULL){return 1;}
	if(Length>UART_MSG_MAX_LEN){return 1;}
	
	uint8_t CFGBuf[3]={0};
	CFGBuf[0] = Length>>8;
	CFGBuf[1] = Length&0xFF;
	
	rt_enter_critical();
	rt_mq_send(&UART0_TxMSG,CFGBuf,UART_MSG_CFG_LEN);
	rt_mq_send(&UART0_TxMSG,pdata,Length);
	rt_exit_critical();
	
	return 0;
}

/*
	������Ϣ���ͽ���,�а�ͷ
	������FIFO�߼���Ӧ
	Len:ע�� ���Ϊ Min(32768,UART_MSG_MAX_LEN)
*/
uint8_t cMSG::MSGTx(uint8_t Head, uint8_t *pdata, uint16_t Length)
{
	if(pdata==NULL){return 1;}
	if(Length>UART_MSG_MAX_LEN){return 1;}
	
	uint8_t CFGBuf[3]={0};
	CFGBuf[0] = Length>>8;
	CFGBuf[0] |=  0x80;
	CFGBuf[1] = Length&0xFF;
	CFGBuf[2] = Head;
	
	rt_enter_critical();
	rt_mq_send(&UART0_TxMSG,CFGBuf,UART_MSG_CFG_LEN);
	rt_mq_send(&UART0_TxMSG,pdata,Length);
	rt_exit_critical();
	
	return 0;
}

/*
	������Ϣ���ͽ���,�ް�ͷ
	��Ϣ���������͵����Ͷ���λ��
	Len:ע�� ���Ϊ Min(32768,UART_MSG_MAX_LEN)
*/
uint8_t cMSG::MSGUrgentTx(uint8_t *pdata, uint16_t Length)
{
	if(pdata==NULL){return 1;}
	if(Length>UART_MSG_MAX_LEN){return 1;}
	
	uint8_t CFGBuf[3]={0};
	CFGBuf[0] = Length>>8;
	CFGBuf[1] = Length&0xFF;
	
	rt_enter_critical();
	rt_mq_urgent(&UART0_TxMSG,pdata,Length);
	rt_mq_urgent(&UART0_TxMSG,CFGBuf,UART_MSG_CFG_LEN);
	rt_exit_critical();
	
	return 0;
}

/*
	������Ϣ���ͽ���,�а�ͷ
	��Ϣ���������͵����Ͷ���λ��
	Len:ע�� ���Ϊ Min(32768,UART_MSG_MAX_LEN)
*/
uint8_t cMSG::MSGUrgentTx(uint8_t Head, uint8_t *pdata, uint16_t Length)
{
	if(pdata==NULL){return 1;}
	if(Length>UART_MSG_MAX_LEN){return 1;}
	
	uint8_t CFGBuf[3]={0};
	CFGBuf[0] = Length>>8;
	CFGBuf[0] |=  0x80;
	CFGBuf[1] = Length&0xFF;
	CFGBuf[2] = Head;
	
	rt_enter_critical();
	rt_mq_send(&UART0_TxMSG,pdata,Length);
	rt_mq_send(&UART0_TxMSG,CFGBuf,UART_MSG_CFG_LEN);
	rt_exit_critical();
	
	return 0;
}