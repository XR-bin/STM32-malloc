#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "malloc.h"

int main(void)
{
    uint8_t *p;

    SysTick_Init();       /* 延时初始化 */
    USART1_Init(115200);  /* 串口1初始化 115200 */
    LED_Init();           /* LED初始化 */
    my_mem_init(0);       /* 使用内部SRAM作为内存管理，并初始化 */

    printf("内存利用率:%d%%\r\n",my_mem_perused(0));

    p = mymalloc(0,3000); /* 在SRAM的内存管理中申请3000字节空间 */
    sprintf((char*)p,"123你好呀，好好好啊");
    printf("\r\n首次申请空间\r\n");
    printf("%s\r\n",p);
    printf("p空间的首地址%d\r\n",(uint32_t)p);
    printf("内存利用率:%d%%\r\n",my_mem_perused(0));

    p = myrealloc(0, p, 4000);
    printf("\r\n重新申请空间\r\n");
    printf("%s\r\n",p);
    printf("p空间的首地址%d\r\n",(uint32_t)p);
    printf("内存利用率:%d%%\r\n",my_mem_perused(0));

    myfree(0, p);         /* 释放p申请的空间 */
    printf("\r\n释放空间\r\n");
    printf("内存利用率:%d%%\r\n",my_mem_perused(0));

    /* 看看空间是否被重新利用 */
    p = mymalloc(0,3000); /* 在SRAM的内存管理中申请3000字节空间 */
    printf("\r\n再次申请空间\r\n");
    printf("p[0] = %d\r\n",(uint8_t)p[0]);
    printf("p空间的首地址%d\r\n",(uint32_t)p);
    printf("内存利用率:%d%%\r\n",my_mem_perused(0));

    while(1)
    {
        LED0_ON;
        LED1_ON;
        delay_ms(300);
        LED0_OFF;
        LED1_OFF;
        delay_ms(300);
    }
}


