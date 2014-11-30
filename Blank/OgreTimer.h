#ifndef __Win32Timer_H__
#define __Win32Timer_H__

#include <ctime>
#include <limits>
#include "windows.h"

/*
ȡ��windef.h�е����¶���
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

//ÿ200��ʹ�ø߾��ȼ�ʱ����ѯ������һ�λ�����ǰ��Ƶ��
#define FREQUENCY_RESAMPLE_RATE 200

//��ʱ��
class Timer
{
private:
	clock_t zeroClock;			//�ο�ʱ��㣬���ڵ;���

    DWORD mStartTick;			//�ο�ʱ��㣬���ڸ߾��ȵ�������

	LONGLONG mLastTime;			//�ϴβ�ѯʱ���������ú󾭹���ʱ�䣬���ڸ߾���
    LARGE_INTEGER mStartTime;	//��ʼʱ�䣬���ڸ߾���
    LARGE_INTEGER mFrequency;	//ʱ��Ƶ�ʣ����ڸ߾���
    DWORD mQueryCount;			//��ѯʱ��Ĵ�����ÿ�ε���get*()�����ֵ����1

    DWORD mProcMask;			//��������Ե����
    DWORD mSysMask;				//ϵͳ��Ե����
    HANDLE mThread;				//���浱ǰ�߳�
public:
	//���캯��������ʱ���
	Timer();
	~Timer();

	/** Resets timer 
		���ü�ʱ���Ĳο�ʱ��Ϊ��ǰ��ʱ��*/
	void reset();

	//���º���ʹ�ø߾��ȼ�ʱ��
	/** Returns milliseconds since initialisation or last reset 
		�������ú󾭹���ʱ�䣬�Ժ���Ϊ��λ���߾��ȣ������10ms*/
	unsigned long getMilliseconds();
	/** Returns microseconds since initialisation or last reset 
		�������ú󾭹���ʱ�䣬��΢��Ϊ��λ���߾��ȣ������10ms*/
	unsigned long getMicroseconds();

	//���º���ʹ�õ;��ȼ�ʱ��
	/** Returns milliseconds since initialisation or last reset, only CPU time measured 
		�������ú󾭹���ʱ�䣬�Ժ���Ϊ��λ���;��ȣ������55ms*/	
	unsigned long getMillisecondsCPU();
	/** Returns microseconds since initialisation or last reset, only CPU time measured 
		�������ú󾭹���ʱ�䣬��΢��Ϊ��λ���;��ȣ������55ms*/	
	unsigned long getMicrosecondsCPU();
};

#endif
