/*----------------------------------------------+
 |												|
 |	ivPlatform.h - Platform Config				|
 |												|
 |		Platform: Win32 (X86)					|
 |												|
 |		Copyright (c) 1999-2009, iFLYTEK Ltd.	|
 |		All rights reserved.					|
 |												|
 +----------------------------------------------*/


/*
 *	TODO: 在这里包含目标平台程序需要的公共头文件
 */

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <crtdbg.h>



/*
 *	TODO: 根据目标平台特性修改下面的配置选项
 */

#define IV_PTR_GRID				4			/* 最大指针对齐值 */

#define IV_PTR_PREFIX						/* 指针修饰关键字(典型取值有 near | far, 可以为空) */
#define IV_CONST				const		/* 常量关键字(可以为空) */
#define IV_STATIC				static		/* 静态函数关键字(可以为空) */
#define IV_INLINE				//__inline	/* 内联关键字(典型取值有 inline, 可以为空) */
#define IV_CALL_STANDARD		//__stdcall	/* 普通函数修饰关键字(典型取值有 stdcall | fastcall | pascal, 可以为空) */

#define IV_TYPE_INT8			char		/* 8位数据类型 */
#define IV_TYPE_INT16			short		/* 16位数据类型 */
#define IV_TYPE_INT32			long		/* 32位数据类型 */

#if 0 /* 48/64 位数据类型是可选的, 如非必要则不要定义, 在某些 32 位平台下, 使用模拟方式提供的 48/64 位数据类型运算效率很低 */
#define IV_TYPE_INT48			__int64		/* 48位数据类型 */
#define IV_TYPE_INT64			__int64		/* 64位数据类型 */
#endif

#define IV_TYPE_SIZE			unsigned long		/* 大小数据类型 */

#define IV_ANSI_MEMORY			0			/* 是否使用 ANSI 内存操作库 */

#define IV_ASSERT(exp)			//_ASSERT(exp)/* 断言操作(可以为空) */
#define IV_YIELD				Sleep(0)	/* 空闲操作(在协作式调度系统中应定义为任务切换调用, 可以为空) */

/* 根据平台编译选项决定是否支持调试 */
#if defined(DEBUG) || defined(_DEBUG)
	#define IV_DEBUG			1			/* 是否支持调试 */
#else
	#define IV_DEBUG			0			/* 是否支持调试 */
#endif

/* 根据平台编译选项决定是否以 Unicode 方式构建 */
#if defined(UNICODE) || defined(_UNICODE)
	#define IV_UNICODE			1			/* 是否以 Unicode 方式构建 */
#else
	#define IV_UNICODE			0			/* 是否以 Unicode 方式构建 */
#endif
