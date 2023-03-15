#ifndef IMU_H
#define IMU_H
#include <main.h>
#include "ICM42688.h"
#include "PID.h"
#ifdef __cplusplus
class cIMU :public cICM42688,public PID_Inc_C
{
	public:
	int16_t GyroCal[3]={0};
	int16_t AccelCal[3]={0};
	void PID_Init(void)
	{
		this->Ref = 41.0f;
		this->Kp = 150.0f;
		this->Ki = 3.0f;
		this->Kd = 2.0f;
		this->IN_RANGE_EN_D = 2.0f;	//����΢���Χ ֵΪ0ʱʼ�տ���
		this->IN_RANGE_EN_I = 0.0f;	//���������Χ ֵΪ0ʱʼ�տ���
		this->MaxOutValue=999.0;		//����޷�
		this->MinOutValue=200.0;
	}
	cIMU()
	{PID_Init();}
};
extern cIMU *IMU;

extern "C" {
void EXTI3_IRQHandler(void);
void EXTI4_IRQHandler(void);

}
#endif
#endif