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
	zeroClock = clock();						//��¼��ǰʱ��

    QueryPerformanceFrequency(&mFrequency);		//��¼����ʱ��Ƶ��
    QueryPerformanceCounter(&mStartTime);		//��¼��ʼʱ��
    mStartTick = GetTickCount();				//����ϵͳ��ʼ���е����ڵ�ʱ�䣬�Ժ������
    mLastTime = 0;
	mQueryCount = 0;

    // Save the current process
	//���浱ǰ����
    HANDLE mProc = GetCurrentProcess();

    // Get the current Affinity
	//���ص�ǰ���̵ġ���Ե�ԡ������ǳ��ܹ�����ĳ�����̵�CPU��
	//Ϊ��ǰ���̵���Ե���ڶ�CPU�Ļ����У��������������������̵�CPU
#if _MSC_VER >= 1400 && defined (_M_X64)
	GetProcessAffinityMask(mProc, (PDWORD_PTR)&mProcMask, (PDWORD_PTR)&mSysMask);
#else
	GetProcessAffinityMask(mProc, &mProcMask, &mSysMask);
#endif
	//���浱ǰ�߳�
    mThread = GetCurrentThread();
}

//-------------------------------------------------------------------------
/*
���س�ʼ���󾭹���ʱ�䣬���ο�ʱ���ļ�����Ժ���Ϊ��λ
��������Ƚ��ң��������һ��ͼ��������������Ĺ�ϵ
ԭ����΢��Ķ�ý��������������������ǰ��һ��ʱ��
����40�ڴε�ѭ����ȡ40�ڴ�ʱ��,��ʱ8299s=2Сʱ18��,��������������,�����������Ǳ�Ҫ��
*/
unsigned long Timer::getMilliseconds()
{
    LARGE_INTEGER curTime;

	//���´����ʾֻʹ�õ�һ��CPU���в�ѯ
    // Set affinity to the first core
	//�����߳���ԵCPUΪ��һ����
    SetThreadAffinityMask(mThread, 1);
    // Query the timer
	//��ѯʱ��
    QueryPerformanceCounter(&curTime);
    // Reset affinity
	//�����̵߳���ԵCPU
    SetThreadAffinityMask(mThread, mProcMask);

	// Resample the frequency
	//����̶����ô���󣬸��²���Ƶ��
    mQueryCount++;
    if(mQueryCount == FREQUENCY_RESAMPLE_RATE)
    {
        mQueryCount = 0;
        QueryPerformanceFrequency(&mFrequency);
    }

    LONGLONG newTime = curTime.QuadPart - mStartTime.QuadPart;
    
    // scale by 1000 for milliseconds
	//�Ŵ�1000������Ϊ�Ժ���Ϊ��λ
    unsigned long newTicks = (unsigned long) (1000 * newTime / mFrequency.QuadPart);

    // detect and compensate for performance counter leaps
    // (surprisingly common, see Microsoft KB: Q274323)
	//���Ժ͸�����ý���������������΢��Ľ��ͣ���ý���ʱ������ͻȻ��ǰ��һ��ʱ�䣬�ǳ����˾��ȣ�
    unsigned long check = GetTickCount() - mStartTick;
    signed long msecOff = (signed long)(newTicks - check);
    if (msecOff < -100 || msecOff > 100)
    {
        // We must keep the timer running forward :)
		//����������Բο�����е������Ա�����ʱ��ľ���
        LONGLONG adjust = (std::min)(msecOff * mFrequency.QuadPart / 1000, newTime - mLastTime);
        mStartTime.QuadPart += adjust;
        newTime -= adjust;

        // Re-calculate milliseconds
		//���¼����ʼ���󾭹���ʱ��
        newTicks = (unsigned long) (1000 * newTime / mFrequency.QuadPart);
    }

    // Record last time for adjust
	//��¼��һ�ε�ʱ��
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
	//�����Ժ���Ϊ��λ��ʱ��㣬�Ա��������
    unsigned long newTicks = (unsigned long) (1000 * newTime / mFrequency.QuadPart);
    
    // detect and compensate for performance counter leaps
    // (surprisingly common, see Microsoft KB: Q274323)
	//���Ժ͸�����ý������������
    unsigned long check = GetTickCount() - mStartTick;
    signed long msecOff = (signed long)(newTicks - check);
    if (msecOff < -100 || msecOff > 100)
    {
        // We must keep the timer running forward :)
		//����������Բο�����е������Ա�����ʱ��ľ���
        LONGLONG adjust = (std::min)(msecOff * mFrequency.QuadPart / 1000, newTime - mLastTime);
        mStartTime.QuadPart += adjust;
        newTime -= adjust;
    }

    // Record last time for adjust
	//��¼��һ�ε�ʱ��
    mLastTime = newTime;

    // scale by 1000000 for microseconds
	//�Ŵ�1000000������Ϊ��΢��Ϊ��λ
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
