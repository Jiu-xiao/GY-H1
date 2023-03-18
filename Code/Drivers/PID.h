/*********************************************************************************
  *FileName:		PID.h
  *Author:  		qianwan
  *Detail: 			PID�㷨����,PID_InitΪ���麯��
  
  *Version:  		1.2
  *Date:  			2023/03/13
  *Describe:		�޲�BUG
  
  *Version:  		1.1
  *Date:  			2023/02/26
  *Describe:		��������΢�ַ�Χ�ж��������ʽ

  *Version:  		1.0
  *Date:  			2023/02/10
  *Describe:		���ڲ�������,ȡ����CMSIS-DSP������
**********************************************************************************/
#ifndef PID_H
#define PID_H
#include "main.h"
#ifdef __cplusplus
class PID_Inc_C
{
	//����ʽ
	public:
	virtual void PID_Init() = 0;
	float PID_Cal(float fdb);
	float	Ref = 0.0f;
	float 	Error = 0.0f; //���
	protected:
	float 	FeedBack = 0.0f;	
	float 	DError = 0.0f;
	float 	DDError = 0.0f;
	float 	PreError = 0.0f;
	float 	PreDError = 0.0f;
		
	float Kp=0.0f; //pid����
	float Ki=0.0f;
	float Kd=0.0f;
	
	float IN_RANGE_EN_D = 0.0f;//����΢���Χ ֵΪ0ʱʼ�տ���
	float IN_RANGE_EN_I = 0.0f;//���������Χ ֵΪ0ʱʼ�տ���
	
	float MaxOutValue=0; //����޷�
	float MinOutValue=0;
	
	public:
	float Out = 0.0f; //���ֵ
};

class PID_Pla_C
{
	//λ��ʽ
	public:
	virtual void PID_Init() = 0;
	float PID_Cal(float fdb);
	float	Ref = 0.0f;
	float 	Error = 0.0f; //���
	protected:
	float 	FeedBack = 0.0f;	
	float 	DError = 0.0f;

	float integral = 0.0f;//���Ļ���
	
	float Kp = 0.0f; //pid����
	float Ki = 0.0f;
	float Kd = 0.0f;
	
	float IN_RANGE_EN_D = 0.0f;//����΢���Χ ֵΪ0ʱʼ�տ���
	float IN_RANGE_EN_I = 0.0f;//���������Χ ֵΪ0ʱʼ�տ���
	
	float MaxOutValue = 0; //����޷�
	float MinOutValue = 0;

	float Maxintegral = 0.0f; //�����޷�
	float Minintegral = 0.0f;
	
	public:
	float Out = 0.0f; //���ֵ
};

#endif
#endif