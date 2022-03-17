#include "sys.h"

/*****************************************************
*函数功能  ：对LED对应的GPIO口进行初始化设置
*函数名    ：LED_Init
*函数参数  ：void
*函数返回值：void
*描述      ：
*            LED0     PB5
*            LED1     PE5
*            高电平灭，低电平亮-----输出模式
********************************************************/
void LED_Init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;               //GPIOx配置结构体

	//时钟使能   GPIOB   GPIOC 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB端口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	 //使能PE端口时钟

	//PB5
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				     //PB5 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					     //根据设定参数初始化PB5
	
	//PE5
	//应其部分配置与PB5一致，所以结构体一些变量不再配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	    		     //PE5 端口配置, 推挽输出
	GPIO_Init(GPIOE, &GPIO_InitStructure);	  				     //根据设定参数初始化PE5
	
	//端口输出数据寄存器配置   //LED灯的初始状态
	GPIO_SetBits(GPIOB,GPIO_Pin_5);						             //PB5 输出高
	GPIO_SetBits(GPIOE,GPIO_Pin_5); 						           //PE5 输出高 
}




