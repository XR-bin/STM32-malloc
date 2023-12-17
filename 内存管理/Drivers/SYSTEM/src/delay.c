#include "delay.h"



/**********************************************************
* @funcName ：SysTick_Init
* @brief    ：滴答定时器初始化
* @param    ：void
* @retval   ：void
* @details  ：
* @fn       ：
************************************************************/
void SysTick_Init(void)
{
    SysTick->CTRL = 0;         /* 清Systick状态，以便下一步重设，如果这里开了中断会关闭其中断 */
    SysTick->CTRL &= ~(1<<2);  /* 选择时钟源   写0是外部时钟源   写1是内部时钟源 */
}



/**********************************************************
* @funcName ：delay_us
* @brief    ：系统滴答定时器微秒延时
* @param    ：uint32_t us (微秒)
* @retval   ：void
* @details  ：
* @fn       ：
************************************************************/
void delay_us(uint32_t us)
{
    SysTick->VAL = 0xff;                 /* 将当前值寄存器清零   随便写一个数据进入 */
    SysTick->LOAD = 9*us;                /* 将要计时的数据写入到重装载值寄存器 */
    SysTick->CTRL |= (1<<0);             /* 开始计数   开计数器使能 */
    while(!(SysTick->CTRL & (1<<16)));   /* 等待计数完成    while(16位为0) */
    SysTick->CTRL &= ~(1<<0);            /* 关闭计数器 */
}



/**********************************************************
* @funcName ：delay1_ms
* @brief    ：系统滴答定时器微秒延时
* @param    ：uint16_t ms (毫秒)
* @retval   ：void
* @details  ：
* @fn       ：
************************************************************/
void delay1_ms(uint16_t ms)
{
    SysTick->VAL = 0xff;                 /* 将当前值寄存器清零   随便写一个数据进入 */
    SysTick->LOAD = 9*1000*ms;           /* 将要计时的数据写入到重装载值寄存器 */ 
    SysTick->CTRL |= (1<<0);             /* 开始计数   开计数器使能 */
    while(!(SysTick->CTRL & (1<<16)));   /* 等待计数完成    while(16位为0) */
    SysTick->CTRL &= ~(1<<0);            /* 关闭计数器 */
}



/**********************************************************
* @funcName ：delay_ms
* @brief    ：系统滴答定时器微秒延时(修改版)
* @param    ：uint16_t ms (毫秒)
* @retval   ：void
* @details  ：
* @fn       ：
************************************************************/
void delay_ms(uint16_t ms)
{
    uint16_t i,n=0;

    if(ms <= 1864)
    {
        delay1_ms(ms);
    }
    else
    {
        while(ms >= 1864)
        {
            ms -= 1864;
            n++;
        }
        for(i=0;i<n;i++) delay1_ms(1864);
        delay1_ms(ms);
    }
}

