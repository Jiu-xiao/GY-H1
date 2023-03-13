#include "MsgThread.h"
#include <stdio.h>


/*���򴮿ڷ��Ϳ���*/
cMSG  *Msg;

usb_dev *my_usb_dev = RT_NULL;
/*���ڽ��̿���ָ��*/
rt_thread_t UART_thread = RT_NULL;
/*������ɿ����ź���*/
rt_sem_t UART0_TxSem = RT_NULL;
/*������ɿ����ź���*/
rt_sem_t UART0_RxSem = RT_NULL;
/*���Ϳ��ƻ�����*/
rt_mutex_t UART0_TxMut = RT_NULL;

/*USB���̿���ָ��*/
rt_thread_t USBD_thread = RT_NULL;
/*USB�жϿ����ź���*/
rt_sem_t USBD_Sem = RT_NULL;
/*USBD���ջ�����*/
uint8_t *USBD_rev_buf;

/*
	���ڿ��ƽ���
	���ƴ�����Ϣ�Ľ���
	����Ӳ����Դ�ĳ�ʼ���ڴ˽��У����ڽ�����Ӧ��Ҫ����Ӳ����Դ��ʼ��ִ��
*/
void UARTThread(void* parameter)
{
	uint8_t IsHead = 0;//�Ƿ���Ҫ��Ӱ�ͷ
	uint16_t TxLen = 0;//���ݲ��ֳ���

	Msg = new cMSG;
	Msg->UART_Init(USART0,DMA0,DMA_CH4,DMA0,DMA_CH3);
	for(;;)
	{
		/*�ȴ��������*/
		rt_sem_take(UART0_RxSem,RT_WAITING_FOREVER);
	}
}

void DMA0_Channel3_IRQHandler(void)
{
	if(Msg->Transmit_IRQ()){
	rt_sem_release(UART0_TxSem);
	}
}


void USBDThread(void* parameter)
{
	my_usb_dev = new usb_dev;
	usbd_init(my_usb_dev,&cdc_desc,&cdc_class);
	Msg->USB_COM = (usb_cdc_handler *)my_usb_dev->class_data[CDC_COM_INTERFACE];
	usbd_connect(my_usb_dev);
	/* wait for standard USB enumeration is finished */
    if(my_usb_dev->cur_status == USBD_CONNECTED){rt_kprintf("USB Connective");}
	for(;;)
	{
		rt_sem_take(USBD_Sem,RT_WAITING_FOREVER);
		Msg->USB_COM = (usb_cdc_handler *)my_usb_dev->class_data[CDC_COM_INTERFACE];
		usbd_ep_send(my_usb_dev, CDC_IN_EP, Msg->USB_COM->data, Msg->USB_COM->receive_length);
		rt_kprintf("\n\nUSBRec Len = %d\n",Msg->USB_COM->receive_length);
		Msg->UartTx(Msg->USB_COM->data,Msg->USB_COM->receive_length);
		rt_kprintf("\n\n");
	}
}

void USBD_LP_CAN0_RX0_IRQHandler(void)
{
	usbd_isr();
    
}

void USBD_WKUP_IRQHandler(void)
{
    exti_interrupt_flag_clear(EXTI_18);
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
uint8_t cMSG::UartTx(uint8_t *pdata, uint16_t Length)
{
	if(pdata==0){return 1;}
	/*ֱ��ʹ��DMA����*/
	if(rt_mutex_take(UART0_TxMut,RT_WAITING_NO)!=RT_EOK){return 1;}
	this->Transmit_DMA(pdata,Length);
	rt_sem_take(UART0_TxSem,RT_WAITING_FOREVER);
	rt_mutex_release(UART0_TxMut);
	return 0;
}
/*
	�����溯���Ļ����ϣ����һ�����ݰ�ͷ
*/
uint8_t cMSG::UartTx(uint8_t Head, uint8_t *pdata, uint16_t Length)
{
	if(pdata==0){return 1;}
	/*ֱ��ʹ��DMA����*/
	if(rt_mutex_take(UART0_TxMut,RT_WAITING_NO)!=RT_EOK){return 1;}
	usart_data_transmit(USART0,Head);
	while (!usart_flag_get(USART0, USART_FLAG_TBE)){}
	this->Transmit_DMA(pdata,Length);
	rt_sem_take(UART0_TxSem,RT_WAITING_FOREVER);
	rt_mutex_release(UART0_TxMut);
	return 0;
}
void cMSG::Printf(const char * format, ...)
{
	char* buf = (char*)rt_malloc(128);

	va_list args;
	va_start(args, format);
	uint8_t Len = vsnprintf(buf,128,format, args);
	va_end (args);
	this->UartTx((uint8_t*)buf,Len);
	rt_free(buf);
}

void rt_hw_console_output(const char *str)
{
	#ifdef qwDbug
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
	#else
	uint16_t i=0;
	while(*(str+i) != '\0'){i++;}
	Msg->UartTx((uint8_t *)str,i);
	#endif
}