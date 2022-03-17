#ifndef _MOLLOC_H
#define _MOLLOC_H

	#include "sys.h"
	
	#ifndef NULL
		#define NULL 0
	#endif

	//定义两个内存池
	#define SRAMIN	 0		//内部内存池
	#define SRAMEX   1		//外部内存池(精英STM32开发板不支持外部内存) 
  //我们又多少个SRAM可管理
	#define SRAMBANK 	2	//定义支持的SRAM块数. 精英版实际上只支持1个内存区域,即内部内存.	
	
	//mem1内存参数设定.mem1完全处于内部SRAM里面.(设置内部SARM的内存池和内存表的参数)
	#define MEM1_BLOCK_SIZE			32  	  						              //一个内存块大小为32字节
	#define MEM1_MAX_SIZE			40*1024  						                //最大管理内存 40K  (我们这个内存管理系统的内部SRAM可控制的内存大小)
	#define MEM1_ALLOC_TABLE_SIZE	MEM1_MAX_SIZE/MEM1_BLOCK_SIZE 	//内存表大小(有多少块内存块)

	//mem2内存参数设定.mem2的内存池处于外部SRAM里面
	#define MEM2_BLOCK_SIZE			32  	  						              //一个内存块大小为32字节
	#define MEM2_MAX_SIZE			1 *32  							                //因为精英版没有外扩内存,故这里设置一个最小值
	#define MEM2_ALLOC_TABLE_SIZE	MEM2_MAX_SIZE/MEM2_BLOCK_SIZE 	//内存表大小 
		
	//内存管理控制器结构体
	//注意：内存管理由内存池和内存列表组成
	//SRAMBANK：SARM块数，一般有内部SRAM和外部SRAM、CCM
	struct _m_mallco_dev
	{
		void (*init)(u8);					    //初始化
		u8 (*perused)(u8);		  	    //内存使用率
		u8 	*membase[SRAMBANK];				//内存池 管理SRAMBANK个区域的内存
		u16 *memmap[SRAMBANK]; 				//内存管理状态表
		u8  memrdy[SRAMBANK]; 				//内存管理是否就绪
	};
	
	void my_mem_init(u8 memx);
	u8 my_mem_perused(u8 memx);

	void *mymalloc(u8 memx,u32 size);
	void myfree(u8 memx,void *ptr);
	void *myrealloc(u8 memx,void *ptr,u32 size);
	
#endif











