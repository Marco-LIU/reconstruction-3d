#ifndef _USB_CAMERA_H_
#define _USB_CAMERA_H_

#include <iostream>
#include <fstream>
#include <vector>

#include "cv.h"
#include "highgui.h"
#include "OgreTimer.h"

#include "JHCap.h"
/*
	使用的api为3.3
	这个类，用于封装多个京航摄像头130万带触发摄像头，提供一个统一的接口

	对于单摄像头，也支持多种类型的摄像头。
*/

//摄像机信息结果
struct CameraInfos
{
	int id;					//自定义的编号
	std::string sn;			//序列号
	int index;				//usb索引
};

//封装一个类，管理所有的摄像头
class UsbCamera
{
//这些数据只为多线程同步捕获
public:
	static int g_result1,g_result2;
	static unsigned char* g_buffer0;
	static unsigned char* g_buffer1;
	static DWORD WINAPI CaptureThread1(LPVOID n)
	{
		int index = *((int*)n);
		int mBufferLength = 1280*1024;
		//gt11 = gTimer.getMicroseconds();
		g_result1= CameraQueryImage(index,g_buffer0,&mBufferLength,CAMERA_IMAGE_RAW8);
		//gt12 = gTimer.getMicroseconds();
		return g_result1;
	}
	static DWORD WINAPI CaptureThread2(LPVOID n)
	{
		int index = *((int*)n);
		int mBufferLength = 1280*1024;
		//gt21 = gTimer.getMicroseconds();
		g_result2= CameraQueryImage(index,g_buffer1,&mBufferLength,CAMERA_IMAGE_RAW8);
		//gt22 = gTimer.getMicroseconds();
		return g_result2;
	}
//设置属性
public:
	//构造函数，初始化所有的摄像机
	UsbCamera();
	/*
	config文件中可以如下设置摄像头
		#设置摄像头序号
		Camera:{289201322025:0, 289201322018:1}
	*/
	UsbCamera(std::string config);
	~UsbCamera();

	//读写第i个编号摄像头的增益,-1表示失败
	int getGain(int id);
	bool setGain(int id,int gain);
	bool setGainAll(int gain);
	void setAutoGain(bool t = true);

	//读写第i个编号摄像头的曝光
	int getExposure(int id);
	bool setExposure(int id,int expo);
	bool setExposureAll(int expo);
	void setAutoExposure(bool t = true);

	//设置相机的读取区域，用于快速读取数据(w,h必须为4的倍数)
	//ROI的设置很复杂，还涉及到了读取的缓存，todo 以后完善这个功能吧
	bool setROI(int id,int l,int t,int w,int h)
	{
		int index =getCameraIndex(id);
		if(index == -1)
		{
			//todo 抛出异常
		}
		CameraSetROI(index,l,t,w,h);

		return true;
	}

	//返回指定摄像机的索引，-1表示摄像机没有正确初始化
	int getCameraIndex(int id);
	//返回索引指定的摄像机id
	int getCameraId(int index);
	//返回索引指定的摄像机的sn
	std::string getCameraSn(int index);
	//返回系统中可以使用的摄像机的个数

	int getCameraCount();
	//停止并释放摄像机资源

	void stopCamera(int index);
	//todo增加重新连接摄像机
	//停止摄像机 todo 测试可用性
	void stopAllCamera();
	void stopOneCamera(int id);
	//启动摄像机
	void startAllCamera();
	void startOneCamera(int id);
//常规捕获
public:
	//捕获特定摄像头的一帧图像(这个函数，在触发类相机时，可能不返回)
	bool captureOneFrame(int id);
	//为了避免上述问题，使用多线程的方式触发(暂时先这样解决)
	bool captureOneFrameWithTrigger(int id)
	{
		int index =getCameraIndex(id);
		if(index == -1)
		{
			//todo 抛出异常
		}

		//创建等待线程
		g_result1 = 1;
		HANDLE hThrds;
		hThrds = CreateThread(NULL,0,CaptureThread1,(LPVOID)(&index),0,NULL);
		Sleep(1);
		//软件触发
		CameraTriggerShot(index);
		//等待线程执行完毕
		WaitForMultipleObjects(1,&hThrds,TRUE,INFINITE);
		//关闭线程和事件
		CloseHandle(hThrds);

		if(g_result1 ==0)
		{
			//复制数据
			memcpy(buffers[index],g_buffer0,mBufferLength);
			return true;
		}
		else
		{
			return false;
		}
	}
	//依次捕获所有摄像头的图像
	bool captureOneFrameAll();
	//尽量同时捕获2张图像
	bool captureTwoFrame(int id1,int id2);
	//返回捕获的缓存，id表示标识，index表示索引
	unsigned char* getBuffer(int id);
	unsigned char* getBufferByIndex(int index);
	cv::Mat getImage(int id);
	cv::Mat getImageByIndex(int index);
//单触发摄像头捕获,索引为0
public:
	//软件触发
	bool captureOneCameraWithTrigger();
	//常规模式
	bool captureOneCameraWithoutTrigger();
//双触发摄像头同步捕获
public:
	//设置所有相机的触发模式(默认是连续工作模式)
	void setTriggerMode(bool t = true);
	//同步获取2张图像(使用软件触发的方式)
	bool captureTwoFrameSyncSoftware(int n1,int n2);
	//同步获取2张图像(使用硬件触发的方式,需要外部触发，否则会无限制的等待)
	bool captureTwoFrameSyncHardware(int n1,int n2);
	//同步获取2张图像（软件触发，机器内部会硬件同步）
	bool captureTwoFrameSyncSoftControl(int n1,int n2)
	{
		int index1 =getCameraIndex(n1);
		if(index1 == -1)
		{
			//todo 抛出异常
		}
		int index2 =getCameraIndex(n2);
		if(index2 == -1)
		{
			//todo 抛出异常
		}

		//创建2个等待线程
		g_result1 = 1;
		g_result2 = 1;
		HANDLE hThrds[2];
		hThrds[0] = CreateThread(NULL,0,CaptureThread1,(LPVOID)(&index1),0,NULL);
		hThrds[1] = CreateThread(NULL,0,CaptureThread2,(LPVOID)(&index2),0,NULL);
		Sleep(1);
		//软件触发
		CameraTriggerShot(index1);
		//等待线程执行完毕
		WaitForMultipleObjects(2,hThrds,TRUE,INFINITE);
		//关闭线程和事件
		CloseHandle(hThrds[0]);
		CloseHandle(hThrds[1]);
		if(g_result1 ==0 && g_result2==0 )
		{
			//复制数据
			memcpy(buffers[index1],g_buffer0,mBufferLength);
			memcpy(buffers[index2],g_buffer1,mBufferLength);
			return true;
		}
		else
		{
			return false;
		}
	}
	static Timer gTimer;
	static std::vector<int> gAllTimes;
	static int gt11,gt12,gt21,gt22;
	bool captureTwoFrameSyncSoftControlWithTimer(int n1,int n2)
	{
gTimer.reset();
		int index1 =getCameraIndex(n1);
		if(index1 == -1)
		{
			//todo 抛出异常
		}
		int index2 =getCameraIndex(n2);
		if(index2 == -1)
		{
			//todo 抛出异常
		}

		//创建2个等待线程
		g_result1 = 1;
		g_result2 = 1;
		HANDLE hThrds[2];
int t1 = gTimer.getMicroseconds();
		hThrds[0] = CreateThread(NULL,0,CaptureThread1,(LPVOID)(&index1),0,NULL);
		hThrds[1] = CreateThread(NULL,0,CaptureThread2,(LPVOID)(&index2),0,NULL);
int t2= gTimer.getMicroseconds();
		Sleep(1);
int t3 = gTimer.getMicroseconds();
		//软件触发
		CameraTriggerShot(index1);
int t4 = gTimer.getMicroseconds();
		//等待线程执行完毕
		WaitForMultipleObjects(2,hThrds,TRUE,INFINITE);
int t5 = gTimer.getMicroseconds();
		//关闭线程和事件
		CloseHandle(hThrds[0]);
		CloseHandle(hThrds[1]);
int t6 = gTimer.getMicroseconds();
		if(g_result1 ==0 && g_result2==0 )
		{
			//复制数据
			memcpy(buffers[index1],g_buffer0,mBufferLength);
			memcpy(buffers[index2],g_buffer1,mBufferLength);
int t7 = gTimer.getMicroseconds();
//共11项
gAllTimes.push_back(t1);
gAllTimes.push_back(gt11);
gAllTimes.push_back(gt21);
gAllTimes.push_back(t2);
gAllTimes.push_back(t3);
gAllTimes.push_back(t4);
gAllTimes.push_back(gt12);
gAllTimes.push_back(gt22);
gAllTimes.push_back(t5);
gAllTimes.push_back(t6);
gAllTimes.push_back(t7);

			return true;
		}
		else
		{
			return false;
		}
	}
private:
	//初始化所有的摄像机，建立相机标号和usb索引之间的关系
	std::vector<CameraInfos> iniCameraInfos(std::string config="");
private:
	std::vector<CameraInfos> mCamInfo;	//所有相机的信息(索引为相机编号)
	std::vector<unsigned char*> buffers;//所有的缓存(按索引排列)
	int mCamNum;						//可用的摄像机个数
	int mBufferLength;					//缓存大小
	int mWidth;							//宽度
	int mHeight;						//高度
};
#endif