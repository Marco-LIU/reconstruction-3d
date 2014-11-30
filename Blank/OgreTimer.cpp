#include "OgreTimer.h"
#include <algorithm>
//-------------------------------------------------------------------------
Timer::Timer()
{
	reset();
}

//-------------------------------------------------------------------------
Timer::~Timer()
{
}

//-------------------------------------------------------------------------
void Timer::reset()
{
	zeroClock = clock();						//记录当前时间

    QueryPerformanceFrequency(&mFrequency);		//记录机器时钟频率
    QueryPerformanceCounter(&mStartTime);		//记录开始时间
    mStartTick = GetTickCount();				//返回系统开始运行到现在的时间，以毫秒计算
    mLastTime = 0;
	mQueryCount = 0;

    // Save the current process
	//保存当前进程
    HANDLE mProc = GetCurrentProcess();

    // Get the current Affinity
	//返回当前进程的“亲缘性”。我们称能够运行某个进程的CPU，
	//为当前进程的亲缘，在多CPU的机器中，你可以设置运行这个进程的CPU
#if _MSC_VER >= 1400 && defined (_M_X64)
	GetProcessAffinityMask(mProc, (PDWORD_PTR)&mProcMask, (PDWORD_PTR)&mSysMask);
#else
	GetProcessAffinityMask(mProc, &mProcMask, &mSysMask);
#endif
	//保存当前线程
    mThread = GetCurrentThread();
}

//-------------------------------------------------------------------------
/*
返回初始化后经过的时间，或距参考时间点的间隔，以毫秒为单位
这个函数比较乱，建议绘制一个图，理清各个变量的关系
原因是微软的多媒体计数器，可能意外的向前跳一段时间
我用40亿次的循环读取40亿次时间,耗时8299s=2小时18分,出现了两次修正,对于修正还是必要的
*/
unsigned long Timer::getMilliseconds()
{
    LARGE_INTEGER curTime;

	//以下代码表示只使用第一个CPU进行查询
    // Set affinity to the first core
	//设置线程亲缘CPU为第一个核
    SetThreadAffinityMask(mThread, 1);
    // Query the timer
	//查询时间
    QueryPerformanceCounter(&curTime);
    // Reset affinity
	//重置线程的亲缘CPU
    SetThreadAffinityMask(mThread, mProcMask);

	// Resample the frequency
	//到达固定调用次序后，更新采样频率
    mQueryCount++;
    if(mQueryCount == FREQUENCY_RESAMPLE_RATE)
    {
        mQueryCount = 0;
        QueryPerformanceFrequency(&mFrequency);
    }

    LONGLONG newTime = curTime.QuadPart - mStartTime.QuadPart;
    
    // scale by 1000 for milliseconds
	//放大1000倍，因为以毫秒为单位
    unsigned long newTicks = (unsigned long) (1000 * newTime / mFrequency.QuadPart);

    // detect and compensate for performance counter leaps
    // (surprisingly common, see Microsoft KB: Q274323)
	//测试和更正多媒体计数器的误差（根据微软的解释，多媒体计时器可能突然向前跳一段时间，非常让人惊讶）
    unsigned long check = GetTickCount() - mStartTick;
    signed long msecOff = (signed long)(newTicks - check);
    if (msecOff < -100 || msecOff > 100)
    {
        // We must keep the timer running forward :)
		//如果产生误差，对参考点进行调整，以便修正时差的精度
        LONGLONG adjust = (std::min)(msecOff * mFrequency.QuadPart / 1000, newTime - mLastTime);
        mStartTime.QuadPart += adjust;
        newTime -= adjust;

        // Re-calculate milliseconds
		//重新计算初始化后经过的时间
        newTicks = (unsigned long) (1000 * newTime / mFrequency.QuadPart);
    }

    // Record last time for adjust
	//记录上一次的时间
    mLastTime = newTime;

    return newTicks;
}

//-------------------------------------------------------------------------
unsigned long Timer::getMicroseconds()
{
    LARGE_INTEGER curTime;
    QueryPerformanceCounter(&curTime);
    LONGLONG newTime = curTime.QuadPart - mStartTime.QuadPart;
    
    // get milliseconds to check against GetTickCount
	//返回以毫秒为单位的时间点，以便进行修正
    unsigned long newTicks = (unsigned long) (1000 * newTime / mFrequency.QuadPart);
    
    // detect and compensate for performance counter leaps
    // (surprisingly common, see Microsoft KB: Q274323)
	//测试和更正多媒体计数器的误差
    unsigned long check = GetTickCount() - mStartTick;
    signed long msecOff = (signed long)(newTicks - check);
    if (msecOff < -100 || msecOff > 100)
    {
        // We must keep the timer running forward :)
		//如果产生误差，对参考点进行调整，以便修正时差的精度
        LONGLONG adjust = (std::min)(msecOff * mFrequency.QuadPart / 1000, newTime - mLastTime);
        mStartTime.QuadPart += adjust;
        newTime -= adjust;
    }

    // Record last time for adjust
	//记录上一次的时间
    mLastTime = newTime;

    // scale by 1000000 for microseconds
	//放大1000000倍，因为以微妙为单位
    unsigned long newMicro = (unsigned long) (1000000 * newTime / mFrequency.QuadPart);

    return newMicro;
}

//-------------------------------------------------------------------------
unsigned long Timer::getMillisecondsCPU()
{
	clock_t newClock = clock();
	return (unsigned long)((float)(newClock-zeroClock) / ((float)CLOCKS_PER_SEC/1000.0)) ;
}

//-------------------------------------------------------------------------
unsigned long Timer::getMicrosecondsCPU()
{
	clock_t newClock = clock();
	return (unsigned long)((float)(newClock-zeroClock) / ((float)CLOCKS_PER_SEC/1000000.0)) ;
}
