#include "Controller.h"
/*ϵͳ���ý��̿���ָ��*/
rt_thread_t Config_thread = RT_NULL;
rt_mailbox_t Config_mailbox = RT_NULL;

/*
	ϵͳ���ÿ��ƽ���
	���յ����Դ��ڻ���CDC�Ŀ���������޸�ϵͳ����
*/
void ConfigThread(void* parameter)
{
	uint32_t RecBuf = 0;
	for(;;)
	{
		rt_mb_recv(Config_mailbox,(rt_ubase_t*)&RecBuf,RT_WAITING_FOREVER);
		switch(RecBuf>>8)
		{
			case 0x00:
				switch(RecBuf&0xFF)
				{
					case 0x00://����
						NVIC_SystemReset();
						while(1);
					break;
					case 0x01:
						
					break;				
					case 0x02:
						
					break;
					case 0x03:
						
					break;				
				}
			break;
			default:
				
				
			break;
		
		}
	}
}