#include "sys.h"
#include "malloc.h"
#include "stm32f10x.h"


/*****************************************************************************
*                           科普C语言                                         *
******************************************************************************
*__align 作用     ：对齐跟数据在内存中的位置有关，也就是内存字节对齐。有点
*                   难理解，需要通过以下举例来理解
*__attribute__作用：可以设置函数属性、变量属性和类型属性。在这里我们用来绝
*                   对定位地址，即专门指点内存地址
*
*实例一：
*     __align(32) u8 mem2base[MEM2_MAX_SIZE] __attribute__((at(0X68000000)));
*   意思：定义一个数组，数组大小为MEM2_MAX_SIZE，数组所占的空间能被32整除，其数组
*         的起始内存地址为0X68000000。
*   如果MEM2_MAX_SIZE为2，则数组内存空间大小为32字节；如果MEM2_MAX_SIZE为33，则数
*   组内存空间大小为64字节。
*
*实例二：
struct A{
	char a;
	unsigned int b;
	unsigned char c;
	char d;
};

另一个结构体是:
structB{
	char a;
	unsigned int b;
	unsigned char c;
	char d;
}__attribute__((align(8)));

sizeof(A) = 12(内存空间大小12个字节) 
sizeof(B) = 8(内存空间大小8个字节)
********************************************************************************/
//内存池(32字节对齐)
//可控制的内存大小
__align(32) u8 mem1base[MEM1_MAX_SIZE];	   												                   //内部SRAM内存池
__align(32) u8 mem2base[MEM2_MAX_SIZE] __attribute__((at(0X68000000)));					     //外部SRAM内存池
//内存管理表
//可控制的内存控制块个数(每个内存块大小为32字节)
u16 mem1mapbase[MEM1_ALLOC_TABLE_SIZE];													                      //内部SRAM内存池MAP
u16 mem2mapbase[MEM2_ALLOC_TABLE_SIZE] __attribute__((at(0X68000000+MEM2_MAX_SIZE)));	//外部SRAM内存池MAP

//内存管理参数	
//const 定义的变量的值是不允许改变的
//因为有内部SRAM和外部SRAM，所以用数组
const u32 memtblsize[SRAMBANK]={MEM1_ALLOC_TABLE_SIZE,MEM2_ALLOC_TABLE_SIZE};			//内存表大小(即控制多少内存块)
const u32 memblksize[SRAMBANK]={MEM1_BLOCK_SIZE,MEM2_BLOCK_SIZE};						      //内存分块大小(一块内存块占多少字节内存空间)
const u32 memsize[SRAMBANK]={MEM1_MAX_SIZE,MEM2_MAX_SIZE};	                      //内存池大小(即可以分配的内存空间大小)

//内存管理控制器
struct _m_mallco_dev mallco_dev=
{
	my_mem_init,				     //内存初始化
	my_mem_perused,				   //内存使用率
	mem1base,mem2base,			 //内存池
	mem1mapbase,mem2mapbase, //内存管理状态表
	0,0,  		 				       //内存管理未就绪
};

/*******************************************
*函数功能  ：复制内存里面的数据(从一个内存空间里拷贝数据到另一内存空间里)
*函数名    ：mymemcpy
*函数参数  ：void *des   void *src   u32 n
*函数返回值：void
*描述      ：
*            *des ：目标地址
*            *src ：源地址
*              n  ：要复制的长度(字节为单位)
*********************************************/
void mymemcpy(void *des,void *src,u32 n)  
{  
	//一般我们不会对要操作的参数指针进行操作
	//而是通过一个变量指针作为中介，这样是为了出于安全保证
	u8 *xdes=des;
	u8 *xsrc=src; 
	//变量在++之前，则先用，后++
	//变量在++之后，先++，后使用
	while(n--)*xdes++=*xsrc++;  
}  

/*****************************************************
*函数功能  ：设置内存(设置内存空间的值，一般用来对空间清0)
*函数名    ：mymemset
*函数参数  ：void *s    u8 c   u32 count
*函数返回值：void
*描述      ：
*            *s    ：内存首地址
*            c     ：要设置的值
*            count ：需要设置的内存大小(字节为单位)
******************************************************/
void mymemset(void *s,u8 c,u32 count)  
{  
	//一般我们不会对要操作的参数指针进行操作
	//而是通过一个变量指针作为中介，这样是为了出于安全保证
	u8 *xs = s;  
	//变量在++之前，则先用，后++
	//变量在++之后，先++，后使用
	while(count--)*xs++=c;  
}

/*****************************************************************
*函数功能  ：内存管理初始化
*函数名    ：my_mem_init
*函数参数  ：u8 memx
*函数返回值：void
*描述      ：
*           memx:所属内存块，即是内部SRAM还是外部SRAM的内存块
*
*    其实所谓的初始化就是把内存池和内存表(他们的本质就是数组)清0
******************************************************************/
void my_mem_init(u8 memx)  
{  
  mymemset(mallco_dev.memmap[memx], 0,memtblsize[memx]*2);  //内存状态表数据清零  
	mymemset(mallco_dev.membase[memx], 0,memsize[memx]);	    //内存池所有数据清零  
	mallco_dev.memrdy[memx]=1;								                //内存管理初始化OK，即内存池和内存表都清0了  
}  

/*****************************************************************
*函数功能  ：获取内存使用率
*函数名    ：my_mem_perused
*函数参数  ：u8 memx
*函数返回值：void
*描述      ：
*           memx:所属内存块，即是内部SRAM还是外部SRAM的内存块
*
*    是否占用是通过判断mem1mapbase或mem2mapbase的数组成员是否非0，如果
* 非0则被占用，之中数组成员值有一定意义，代表占了多少块，如值为10，则表示
* 该申请了连续10个内存块
******************************************************************/
u8 my_mem_perused(u8 memx)  
{  
	u32 used=0;  
	u32 i;  
  //memtblsize：内存表大小(一共内存块数)
	//遍历内存表数组
	for(i=0;i<memtblsize[memx];i++)  
	{  
		//mallco_dev.memmap[memx]：内存表数组
		//取出每个成员判断是否非0
		//非0则是用了
		if(mallco_dev.memmap[memx][i])used++; 
	} 
	
	//使用数量/数量总数*100 = 使用率
	return (used*100)/(memtblsize[memx]);  
}

/*****************************************************************
*函数功能  ：内存分配(内部调用)------确定在内存池的偏移量
*函数名    ：my_mem_malloc
*函数参数  ：u8 memx,u32 size
*函数返回值：u32
*描述      ：
*           memx:所属内存块，即是内部SRAM还是外部SRAM的内存块
*           size:要分配的内存大小(字节)
*           返回值:0XFFFFFFFF,代表错误;其他,内存偏移地址
*
*           注意：内存表的遍历是从后往前的
******************************************************************/
u32 my_mem_malloc(u8 memx,u32 size)  
{  
	signed long offset=0;  //偏移量变量 
	u32 nmemb;	           //需要的内存块数  
	u32 cmemb=0;           //连续空内存块数，保证我们申请的内存块是连续的
	u32 i;  
	//判断是否已执行了初始化
	if(!mallco_dev.memrdy[memx])mallco_dev.init(memx);//未初始化,先执行初始化 
	if(size==0)return 0XFFFFFFFF;//不需要分配
  //内存块数 = 申请空间大小(字节单位) / t一个内存块大小(字节单位)
	nmemb=size/memblksize[memx];  	//获取需要分配的连续内存块数
	//申请空间大小(字节单位) / t一个内存块大小(字节单位) ！= 0
	//如果非0则要多申请一块内存块
	if(size%memblksize[memx])nmemb++;  
	//内存表的遍历是从后往前的
	for(offset=memtblsize[memx]-1;offset>=0;offset--)//搜索整个内存控制区  
	{ 
    //判断该内存块是否被占用了   
		if(!mallco_dev.memmap[memx][offset])cmemb++;//连续空内存块数增加
		//保证内存块的连续性
		else cmemb=0;								                //连续内存块清零
		//确定好所有内存块位置后
		if(cmemb==nmemb)							              //找到了连续nmemb个空内存块
		{
			for(i=0;i<nmemb;i++)  				     	//标注内存块非空 
			{  
				//开始往内存块在内存表数组的位置标记该内存块被占用
				mallco_dev.memmap[memx][offset+i]=nmemb;  
			} 
			
			//确定申请空间在内存池数组位置    在内存表数组位置*一个内存块大小(32字节) 
			return (offset*memblksize[memx]);   //返回偏移地址  
		}
	}  
	return 0XFFFFFFFF;//未找到符合分配条件的内存块  
}  

/*****************************************************************
*函数功能  ：释放内存(内部调用)------内存池偏移量清除申请空间在内存表的占用标志
*函数名    ：my_mem_free
*函数参数  ：u8 memx,u32 offset
*函数返回值：u32
*描述      ：
*           memx:所属内存块，即是内部SRAM还是外部SRAM的内存块
*           size:内存地址偏移(字节)--------也就是在内存池数组的位置
*           返回值:0,释放成功;1,释放失败;
*
******************************************************************/
u8 my_mem_free(u8 memx,u32 offset)  
{  
	int i;
	int index;
	int nmemb;
	
	//判断是否初始化
	if(!mallco_dev.memrdy[memx])//未初始化,先执行初始化
	{
		mallco_dev.init(memx);    
    return 1;//未初始化  
  }
  
	//判断这个偏移量是否超出了内存池的大小
	if(offset<memsize[memx])//偏移在内存池内. 
	{ 
		//内存表偏移量 = 内存池偏移量/一块内存块大小
		index=offset/memblksize[memx];			  //偏移所在内存块号码  
		//内存表数组成员的值就是申请的块数
		nmemb=mallco_dev.memmap[memx][index];	//内存块数量
		
		for(i=0;i<nmemb;i++)  						        //内存块清零
		{  
			//清除申请空间在内存表的标记
			mallco_dev.memmap[memx][index+i]=0;  
		}  
		return 0;  
	}else return 1;//偏移超区了.  
}  

/*****************************************************************
*函数功能  ：分配内存(外部调用)
*函数名    ：mymalloc
*函数参数  ：u8 memx,u32 size
*函数返回值：void *
*描述      ：
*           memx:所属内存块，即是内部SRAM还是外部SRAM的内存块
*           size:内存大小(字节)
*           返回值:分配到的内存首地址
******************************************************************/
void *mymalloc(u8 memx,u32 size)  
{  
	u32 offset;    //在内存池数组的偏移量变量  
	//获取在内存池数组的偏移量
	offset=my_mem_malloc(memx,size); 
  //如果申请错误，则返回空地址	
	if(offset==0XFFFFFFFF)return NULL;  
	//如果申请成功，则返回申请空间首地址
	else return (void*)((u32)mallco_dev.membase[memx]+offset);  
}  

/*****************************************************************
*函数功能  ：释放内存(外部调用)
*函数名    ：myfree
*函数参数  ：u8 memx,void *ptr
*函数返回值：u32
*描述      ：
*           memx:所属内存块，即是内部SRAM还是外部SRAM的内存块
*           ptr :要释放的内存空间首地址 
******************************************************************/
void myfree(u8 memx,void *ptr)  
{  
	u32 offset;
  u32 n;     //该要释放的空间的空间大小	
	if(ptr==NULL)return;//地址为0. 
  //确定申请空间的内存池偏移量	
 	offset=(u32)ptr-(u32)mallco_dev.membase[memx]; 
	//空间占内存池空间的大小
	n=mallco_dev.memmap[memx][offset/memblksize[memx]] * memblksize[memx];	
	//释放内存池对应空间的数据
	mymemset(ptr,0,n);
  //释放内存表	
  my_mem_free(memx,offset);	//释放内存      
}  


/*****************************************************************
*函数功能  ：重新分配内存(外部调用)
*函数名    ：myfree
*函数参数  ：u8 memx,void *ptr
*函数返回值：u32
*描述      ：
*           memx:所属内存块，即是内部SRAM还是外部SRAM的内存块
*           ptr :旧内存空间地址首地址
*           size:要重新分配的内存大小(字节)
******************************************************************/
void *myrealloc(u8 memx,void *ptr,u32 size)  
{  
	u32 offset; 
	
	//申请一个新的空间
	offset=my_mem_malloc(memx,size);   	
	if(offset==0XFFFFFFFF)return NULL;     
	else  
	{ 
    //把旧空间的数据复制到新空间里		
		mymemcpy((void*)((u32)mallco_dev.membase[memx]+offset),ptr,size);	//拷贝旧内存内容到新内存  
		//删掉旧空间
		myfree(memx,ptr);                                              		//释放旧内存
		//返回新空间地址
		return (void*)((u32)mallco_dev.membase[memx]+offset);  				    //返回新内存首地址
	}    
}



