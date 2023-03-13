/*********************************************************************************
  *FileName:		PID.cpp
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
#include "PID.h"
#include "math.h"
/*����ʽPID*/
float PID_Inc_C::PID_Cal(float fdb)
{
	/*�м���*/
	float Kp =0.0f;
	float Ki =0.0f;
	float Kd =0.0f;
	float OUT =0.0f;
	
	/*ǰ��׼��*/
	this->FeedBack = fdb;
	this->Error 	= this->Ref 	 - this->FeedBack;	
	this->DError 	= this->Error  - this->PreError;
	this->DDError = this->DError - this->PreDError;			
	this->PreError = this->Error;
	this->PreDError = this->DError;
			
	/*��������΢������*/
	//pid->Out = pid->Out + (pid->Kp * pid->DError + pid->Ki * pid->Error + pid->Kd * pid->DDError);
	Kp = this->Kp * this->DError;		
	//I ���ַ���
	if(!IN_RANGE_EN_I?1:(fabs(this->Error)<this->IN_RANGE_EN_I))
	{Ki = this->Ki * this->Error;}		
	//D ΢�ַ���
	if(!IN_RANGE_EN_D?1:(fabs(this->Error)<this->IN_RANGE_EN_D))
	{Kd = this->Kd * this->DDError;}	
	//���
	OUT = this->Out + Kp + Ki + Kd;
	
	/*���ڴ���*/
	//����޷�
	OUT = (OUT > this->MaxOutValue)?this->MaxOutValue:OUT;
	OUT = (OUT < this->MinOutValue)?this->MinOutValue:OUT;
	//��ֵ
	this->Out = OUT;
	return OUT;
}
	
/*λ��ʽPID*/
float PID_Pla_C::PID_Cal(float fdb)
{
	/*�м���*/
	float Kp =0.0f;
	float Ki =0.0f;
	float Kd =0.0f;
	float OUT =0.0f;
	
	/*ǰ��׼��*/
	this->FeedBack = fdb;
	this->Error = this->Ref - this->FeedBack;
	this->integral	+=	this->Error;

	//�����޷�
	this->integral = (this->integral > this->Maxintegral)?this->Maxintegral:this->integral;
	this->integral = (this->integral < this->Minintegral)?this->Minintegral:this->integral;
	
	/*��������΢������*/
	//pid->Out = pid->Kp * pid->Error + pid->Ki * pid->integral  + pid->Kd * (pid->Error - pid->DError);		
	//P	
	Kp = this->Kp * this->Error;		
	//I ���ַ���
	if(!IN_RANGE_EN_I?1:(fabs(this->Error)<this->IN_RANGE_EN_I))	
	{Ki = this->Ki * this->integral;}		
	//D ΢�ַ���
	if(!IN_RANGE_EN_D?1:(fabs(this->Error)<this->IN_RANGE_EN_D))
	{Kd = this->Kd * (this->Error - this->DError);}		
	//���
	OUT = Kp + Ki + Kd;
			
	/*���ڴ���*/
	//����޷�
	OUT = (OUT > this->MaxOutValue)?this->MaxOutValue:OUT;
	OUT = (OUT < this->MinOutValue)?this->MinOutValue:OUT;
	//��ֵ
	this->DError = this->Error;
	this->Out = OUT;
	return OUT;
}
