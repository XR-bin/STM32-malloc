#include "malloc.h"



/**
******************************************************************************
* @file      ：.\Middlewares\Malloc\malloc.c
*              .\Middlewares\Malloc\malloc.h
* @author    ：XRbin
* @version   ：V1.0
* @date      ：2023-12-16
* @brief     ：内存管理代码
******************************************************************************
* @attention
*   我的GitHub   ：https://github.com/XR-bin
*   我的gitee    ：https://gitee.com/xrbin
*   我的leetcode ：https://leetcode.cn/u/xrbin/
******************************************************************************
*/



#if !(__ARMCC_VERSION >= 6010050)   /* 不是AC6编译器，即使用AC5编译器时 */

    /* 内存池(64字节对齐) */
    static __align(64) uint8_t mem1base[MEM1_MAX_SIZE];                                     /* 内部SRAM内存池 */
    static __align(64) uint8_t mem2base[MEM2_MAX_SIZE] __attribute__((at(0X68000000)));     /* 外扩SRAM内存池 */
    /* 内存管理表 */
    static MT_TYPE mem1mapbase[MEM1_ALLOC_TABLE_SIZE];                                                  /* 内部SRAM内存池MAP */
    static MT_TYPE mem2mapbase[MEM2_ALLOC_TABLE_SIZE] __attribute__((at(0X68000000 + MEM2_MAX_SIZE)));  /* 外扩SRAM内存池MAP */

#else   /* 是AC6编译器，使用AC6编译器时 */

    /* mem2base的地址：0x68000000，AC6不支持at宏定义表达式，修改为以下的宏定义表达式！ */
    /* 内存池(64字节对齐) */
    static __ALIGNED(64) uint8_t mem1base[MEM1_MAX_SIZE];                                                       /* 内部SRAM内存池 */
    static __ALIGNED(64) uint8_t mem2base[MEM2_MAX_SIZE] __attribute__((section(".bss.ARM.__at_0X68000000")));  /* 外扩SRAM内存池 */
    /* 内存管理表 */
    static MT_TYPE mem1mapbase[MEM1_ALLOC_TABLE_SIZE];                                                          /* 内部SRAM内存池MAP */
    static MT_TYPE mem2mapbase[MEM2_ALLOC_TABLE_SIZE] __attribute__((section(".bss.ARM.__at_0X680F0C00")));     /* 外扩SRAM内存池MAP */

#endif



/* 内存管理参数 */
const uint32_t memtblsize[SRAMBANK] = {MEM1_ALLOC_TABLE_SIZE, MEM2_ALLOC_TABLE_SIZE};   /* 内存表大小(即控制多少内存块) */
const uint32_t memblksize[SRAMBANK] = {MEM1_BLOCK_SIZE, MEM2_BLOCK_SIZE};               /* 内存分块大小(一块内存块占多少字节内存空间) */
const uint32_t memsize[SRAMBANK]    = {MEM1_MAX_SIZE, MEM2_MAX_SIZE};                   /* 内存池大小(即可以分配的内存空间大小) */



/* 内存管理控制器 */
struct _m_mallco_dev mallco_dev =
{
    my_mem_init,                    /* 内存初始化 */
    my_mem_perused,                 /* 内存使用率 */
    mem1base, mem2base,             /* 内存池 */
    mem1mapbase, mem2mapbase,       /* 内存管理状态表 */
    0, 0,                           /* 内存管理未就绪 */
};



/**********************************************************
* @funcName ：my_memcpy
* @brief    ：复制内存里面的数据(从一个内存空间里拷贝数据到另一内存空间里)
* @param    ：void *des   (目的地址)
* @param    ：void *src   (源地址)
* @param    ：uint32_t n  (需要复制的内存长度(字节为单位))
* @retval   ：void
* @details  ：
* @fn       ：
************************************************************/
void my_memcpy(void *des, void *src, uint32_t n)
{  
    /* 一般我们不会对要操作的参数指针进行操作 */
    /* 而是通过一个变量指针作为中介，这样是为了出于安全保证 */
    uint8_t *xdes = des;
    uint8_t *xsrc = src;

    while(n--) *xdes++ = *xsrc++;
}



/**********************************************************
* @funcName ：my_memset
* @brief    ：设置内存(设置内存空间的值，一般用来对空间清0)
* @param    ：void *s         (内存首地址)
* @param    ：uint8_t c       (要设置的值)
* @param    ：uint32_t count  (需要设置的内存大小(字节为单位))
* @retval   ：void
* @details  ：
* @fn       ：
************************************************************/
void my_memset(void *s, uint8_t c, uint32_t count)
{
    /* 一般我们不会对要操作的参数指针进行操作 */
    /* 而是通过一个变量指针作为中介，这样是为了出于安全保证 */
    uint8_t *xs = s;

    while(count--) *xs ++= c;
}



/**********************************************************
* @funcName ：my_mem_init
* @brief    ：内存管理初始化
* @param    ：uint8_t memx (所属内存块，即是内部SRAM还是外部SRAM的内存块)
* @retval   ：void
* @details  ：
* @fn       ：其实所谓的初始化就是把内存池和内存表(他们的本质就是数组)清0
************************************************************/
void my_mem_init(uint8_t memx)
{
    uint8_t mttsize = sizeof(MT_TYPE);                               /* 获取memmap数组的类型长度(16bit /32bit)*/
    my_memset(mallco_dev.memmap[memx], 0, memtblsize[memx]*mttsize); /* 内存状态表数据清零 */
    mallco_dev.memrdy[memx] = 1;                                     /* 内存管理初始化OK 即内存池和内存表都清0了 */
}



/**********************************************************
* @funcName ：my_mem_perused
* @brief    ：获取内存使用率
* @param    ：uint8_t memx (所属内存块，即是内部SRAM还是外部SRAM的内存块)
* @retval   ：uint16_t --- (使用率，扩大了10倍,0~1000,代表0.0%~100.0%)
* @details  ：
* @fn       ：
*                是否占用是通过判断mem1mapbase或mem2mapbase的数组成
*            员是否非0，如果非0则被占用，之中数组成员值有一定意义，代
*            表占了多少块，如值为10，则表示该申请了连续10个内存块
************************************************************/
uint16_t my_mem_perused(uint8_t memx)
{
    uint32_t used = 0;
    uint32_t i;

    /* memtblsize：内存表大小(一共内存块数) */
    /* 遍历内存表数组 */
    for(i=0;i<memtblsize[memx];i++)
    {
        /* mallco_dev.memmap[memx]：内存表数组 */
        /* 取出每个成员判断是否非0 */
        /* 非0则是用了 */
        if(mallco_dev.memmap[memx][i])used++;
    }

    /* 使用数量/数量总数*100 = 使用率 */
    return (used*100)/(memtblsize[memx]);
}



/**********************************************************
* @funcName ：my_mem_malloc
* @brief    ：内存分配(内部调用)------确定在内存池的偏移量
* @param    ：uint8_t memx  (所属内存块，即是内部SRAM还是外部SRAM的内存块)
* @param    ：uint32_t size (要分配的内存大小(字节))
* @retval   ：uint32_t --- (0XFFFFFFFF,代表错误;其他,内存偏移地址)
* @details  ：
* @fn       ：注意：内存表的遍历是从后往前的
************************************************************/
static uint32_t my_mem_malloc(uint8_t memx, uint32_t size)
{
    signed long offset=0;  /* 偏移量变量 */
    uint32_t nmemb;        /* 需要的内存块数 */
    uint32_t cmemb=0;      /* 连续空内存块数，保证我们申请的内存块是连续的 */
    uint32_t i;

    /* 未初始化,先执行初始化 */
    if(!mallco_dev.memrdy[memx]) mallco_dev.init(memx);

    /* 不需要分配 */
    if(size==0) return 0XFFFFFFFF;

    /* 内存块数 = 申请空间大小(字节单位) / t一个内存块大小(字节单位) */
    nmemb=size/memblksize[memx];   /* 获取需要分配的连续内存块数 */

    /* 申请空间大小(字节单位) / t一个内存块大小(字节单位) ！= 0 */
    /* 如果非0则要多申请一块内存块 */
    if(size%memblksize[memx]) nmemb++;

    /* 内存表的遍历是从后往前的 */
    for(offset=memtblsize[memx]-1;offset>=0;offset--)  /* 搜索整个内存控制区 */
    {
        /* 判断该内存块是否被占用了 */ 
        if(!mallco_dev.memmap[memx][offset])
        {
            cmemb++;            /* 连续空内存块数增加 */
        }
        /* 保证内存块的连续性 */
        else
        {
            cmemb=0;            /* 连续内存块清零 */
        }

        /* 确定好所有内存块位置后 */
        if(cmemb==nmemb)            /* 找到了连续nmemb个空内存块 */
        {
            /* 标注内存块非空 */
            for(i=0;i<nmemb;i++)
            {
                /* 开始往内存块在内存表数组的位置标记该内存块被占用 */
                mallco_dev.memmap[memx][offset+i]=nmemb;
            }

            /* 确定申请空间在内存池数组位置 */
            /* 在内存表数组位置*一个内存块大小(32字节) */
            return (offset*memblksize[memx]);   /* 返回偏移地址 */
        }
    }

    return 0XFFFFFFFF;/* 未找到符合分配条件的内存块 */
}



/**********************************************************
* @funcName ：my_mem_free
* @brief    ：释放内存(内部调用)------内存池偏移量清除申请空间在内存表的占用标志
* @param    ：uint8_t memx    (所属内存块，即是内部SRAM还是外部SRAM的内存块)
* @param    ：uint32_t offset (内存地址偏移(字节),也就是在内存池数组的位置)
* @retval   ：uint8_t --- (0,释放成功; 1,释放失败; 2,超区域了(失败);)
* @details  ：
* @fn       ：
************************************************************/
static uint8_t my_mem_free(uint8_t memx,uint32_t offset)
{
    int i;
    int index;
    int nmemb;

    /* 判断是否初始化 */
    if(!mallco_dev.memrdy[memx])
    {
        mallco_dev.init(memx);
        return 1;
    }

    /* 判断这个偏移量是否超出了内存池的大小 */
    if(offset<memsize[memx])   /* 偏移在内存池内. */
    {
        /* 内存表偏移量 = 内存池偏移量/一块内存块大小 */
        index = offset / memblksize[memx];             /* 偏移所在内存块号码 */
        /* 内存表数组成员的值就是申请的块数 */
        nmemb = mallco_dev.memmap[memx][index];        /* 内存块数量 */

        for(i=0;i<nmemb;i++)                           /* 内存块清零 */
        {
            /* 清除申请空间在内存表的标记 */
            mallco_dev.memmap[memx][index+i] = 0;
        }
        return 0;
    }
    else
    {
        return 2;   /* 偏移超区了. */
    }
}



/**********************************************************
* @funcName ：mymalloc
* @brief    ：分配内存(外部调用)
* @param    ：uint8_t memx  (所属内存块，即是内部SRAM还是外部SRAM的内存块)
* @param    ：uint32_t size (要分配的内存大小(字节))
* @retval   ：void* --- (分配到的内存首地址.)
* @details  ：
* @fn       ：
************************************************************/
void *mymalloc(uint8_t memx, uint32_t size)
{
    uint32_t offset;    /* 在内存池数组的偏移量变量 */

    /* 获取在内存池数组的偏移量 */
    offset=my_mem_malloc(memx,size);

    /* 如果申请错误，则返回空地址 */
    if(offset==0XFFFFFFFF)
    {
        return NULL;
    }
    /* 如果申请成功，则返回申请空间首地址 */
    else
    {
        return (void*)((uint32_t)mallco_dev.membase[memx]+offset);
    }
}



/**********************************************************
* @funcName ：myfree
* @brief    ：释放内存(外部调用)
* @param    ：uint8_t memx  (所属内存块，即是内部SRAM还是外部SRAM的内存块)
* @param    ：void *ptr (要释放的内存空间首地址)
* @retval   ：void
* @details  ：
* @fn       ：
************************************************************/
void myfree(uint8_t memx, void *ptr)
{
    uint32_t offset;

    if(ptr==NULL) return;    /* 地址为0. */

    /* 确定申请空间的内存池偏移量 */
    offset=(uint32_t)ptr-(uint32_t)mallco_dev.membase[memx];

    /* 释放内存表 */
    my_mem_free(memx,offset);     /* 释放内存 */
}



/**********************************************************
* @funcName ：myrealloc
* @brief    ：重新分配内存(外部调用)
* @param    ：uint8_t memx  (所属内存块，即是内部SRAM还是外部SRAM的内存块)
* @param    ：void *ptr     (要释放的内存空间首地址)
* @param    ：uint32_t size (要分配的内存大小(字节))
* @retval   ：void* --- (新分配到的内存首地址.)
* @details  ：
* @fn       ：
************************************************************/
void *myrealloc(uint8_t memx, void *ptr, uint32_t size)  
{  
    uint32_t offset; 

    /* 申请一个新的空间 */
    offset=my_mem_malloc(memx,size);

    if(offset==0XFFFFFFFF)     /* 申请出错 */
    {
        return NULL;           /* 返回空(0) */
    }
    else                       /* 申请没问题, 返回首地址 */
    {
        /* 把旧空间的数据复制到新空间里 */
        my_memcpy((void*)((uint32_t)mallco_dev.membase[memx]+offset), ptr, size);  /* 拷贝旧内存内容到新内存 */
        /* 删掉旧空间 */
        myfree(memx,ptr);        /* 释放旧内存 */
        /* 返回新空间地址 */
        return (void*)((uint32_t)mallco_dev.membase[memx]+offset);   /* 返回新内存首地址 */
    }
}

