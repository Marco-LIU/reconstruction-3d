#ifndef __Win32Timer_H__
#define __Win32Timer_H__

#include <ctime>
#include <limits>
#include "windows.h"

/*
取消windef.h中的如下定义
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
*/
#ifdef max
#  undef max
#endif
#ifdef min
#  undef min
#endif

//每200次使用高精度计时器查询，更新一次机器当前的频率
#define FREQUENCY_RESAMPLE_RATE 200

//计时器
class Timer
{
private:
	clock_t zeroClock;			//参考时间点，用于低精度

    DWORD mStartTick;			//参考时间点，用于高精度的误差调整

	LONGLONG mLastTime;			//上次查询时，距离重置后经过的时间，用于高精度
    LARGE_INTEGER mStartTime;	//开始时间，用于高精度
    LARGE_INTEGER mFrequency;	//时钟频率，用于高精度
    DWORD mQueryCount;			//查询时间的次数，每次调用get*()，这个值增加1

    DWORD mProcMask;			//处理器亲缘掩码
    DWORD mSysMask;				//系统亲缘掩码
    HANDLE mThread;				//保存当前线程
public:
	//构造函数，重置时间点
	Timer();
	~Timer();

	/** Resets timer 
		设置计时器的参考时间为当前的时间*/
	void reset();

	//以下函数使用高精度计时器
	/** Returns milliseconds since initialisation or last reset 
		返回重置后经过的时间，以毫秒为单位，高精度，误差限10ms*/
	unsigned long getMilliseconds();
	/** Returns microseconds since initialisation or last reset 
		返回重置后经过的时间，以微秒为单位，高精度，误差限10ms*/
	unsigned long getMicroseconds();

	//以下函数使用低精度计时器
	/** Returns milliseconds since initialisation or last reset, only CPU time measured 
		返回重置后经过的时间，以毫秒为单位，低精度，误差限55ms*/	
	unsigned long getMillisecondsCPU();
	/** Returns microseconds since initialisation or last reset, only CPU time measured 
		返回重置后经过的时间，以微秒为单位，低精度，误差限55ms*/	
	unsigned long getMicrosecondsCPU();
};

#endif
