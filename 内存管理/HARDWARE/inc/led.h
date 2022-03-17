#ifndef __LED_H
#define __LED_H	 

	#include "sys.h"

	#define LED0_ON  GPIO_ResetBits(GPIOB,GPIO_Pin_5);    //开LED0
	#define LED1_ON  GPIO_ResetBits(GPIOE,GPIO_Pin_5);    //开LED1
	#define LED0_OFF GPIO_SetBits(GPIOB,GPIO_Pin_5);      //关LED0
	#define LED1_OFF GPIO_SetBits(GPIOE,GPIO_Pin_5);      //关LED1

	void LED_Init(void);       //LED初始化

		 				    
#endif


