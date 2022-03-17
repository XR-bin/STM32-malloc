#include "sys.h"	
#include "delay.h"	
#include "led.h"
#include "usart.h"
#include "stdio.h"
#include "malloc.h"

extern u32 IRnum;

int main(void)
{		
	u8 *p;
	
	SysTick_Init();	  	//延时初始化
	USART1_Init(115200);
  LED_Init();
	my_mem_init(0);
	
	p = mymalloc(0,3000);
	
	sprintf((char*)p,"你好呀，好好好啊");
	
	printf("%s\r\n",p);
	printf("内存利用率:%d%%\r\n",my_mem_perused(0));
	while(1)
	{
		
	}	 
}




