#ifndef __DELAY_H
#define __DELAY_H

    /****************   外部头文件声明   ****************/
    #include "sys.h"



    /****************    函数外部声明   *****************/
    void SysTick_Init(void);          /* 滴答定时器初始化 */
    void delay_us(uint32_t us);       /* 微妙延时 */
    void delay1_ms(uint16_t ms);      /* 毫秒延时 */
    void delay_ms(uint16_t ms);       /* 毫秒延时 */

#endif





























