#include "Controller.h"
#include "Flash_GD.h"
#include "IMU.h"
/*����ָ��*/
cCTR *qCtr;

/*ϵͳ���ý��̿���ָ��*/
rt_thread_t Config_thread = RT_NULL;
rt_mailbox_t Config_mailbox = RT_NULL;

/*
	ϵͳ���ÿ��ƽ���
	���յ����Դ��ڻ���CDC�Ŀ���������޸�ϵͳ����
*/
void ConfigThread(void* parameter)
{
	qCtr = new cCTR;
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

void ConfigRead(void)
{
	uint32_t buf[5];
	fmc_read_u32(FLASH_USERDATA_ADDRESS,buf,5);
	
	qCtr->OTSel  	= (uint8_t)(buf[0]>>24);
	qCtr->CAN_ID 	= (uint16_t)(SYS_CAN_ID_BASE+(buf[0]>>8));
	qCtr->ODR 		= (uint8_t)(buf[0]>>8);
	IMU->GyroCal[0] = (int16_t)(buf[1]>>16);
	IMU->GyroCal[1] = (int16_t)(buf[1]&0xFFFF);
	IMU->GyroCal[2] = (int16_t)(buf[2]>>16);
	IMU->AccelCal[0] = (int16_t)(buf[2]&0xFFFF);
	IMU->AccelCal[1] = (int16_t)(buf[3]>>16);
	IMU->AccelCal[2] = (int16_t)(buf[3]&0xFFFF);
}	
