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
	ʹ�õ�apiΪ3.3
	����࣬���ڷ�װ�����������ͷ130�����������ͷ���ṩһ��ͳһ�Ľӿ�

	���ڵ�����ͷ��Ҳ֧�ֶ������͵�����ͷ��
*/

//�������Ϣ���
struct CameraInfos
{
	int id;					//�Զ���ı��
	std::string sn;			//���к�
	int index;				//usb����
};

//��װһ���࣬�������е�����ͷ
class UsbCamera
{
//��Щ����ֻΪ���߳�ͬ������
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
//��������
public:
	//���캯������ʼ�����е������
	UsbCamera();
	/*
	config�ļ��п���������������ͷ
		#��������ͷ���
		Camera:{289201322025:0, 289201322018:1}
	*/
	UsbCamera(std::string config);
	~UsbCamera();

	//��д��i���������ͷ������,-1��ʾʧ��
	int getGain(int id);
	bool setGain(int id,int gain);
	bool setGainAll(int gain);
	void setAutoGain(bool t = true);

	//��д��i���������ͷ���ع�
	int getExposure(int id);
	bool setExposure(int id,int expo);
	bool setExposureAll(int expo);
	void setAutoExposure(bool t = true);

	//��������Ķ�ȡ�������ڿ��ٶ�ȡ����(w,h����Ϊ4�ı���)
	//ROI�����úܸ��ӣ����漰���˶�ȡ�Ļ��棬todo �Ժ�����������ܰ�
	bool setROI(int id,int l,int t,int w,int h)
	{
		int index =getCameraIndex(id);
		if(index == -1)
		{
			//todo �׳��쳣
		}
		CameraSetROI(index,l,t,w,h);

		return true;
	}

	//����ָ���������������-1��ʾ�����û����ȷ��ʼ��
	int getCameraIndex(int id);
	//��������ָ���������id
	int getCameraId(int index);
	//��������ָ�����������sn
	std::string getCameraSn(int index);
	//����ϵͳ�п���ʹ�õ�������ĸ���

	int getCameraCount();
	//ֹͣ���ͷ��������Դ

	void stopCamera(int index);
	//todo�����������������
	//ֹͣ����� todo ���Կ�����
	void stopAllCamera();
	void stopOneCamera(int id);
	//���������
	void startAllCamera();
	void startOneCamera(int id);
//���沶��
public:
	//�����ض�����ͷ��һ֡ͼ��(����������ڴ��������ʱ�����ܲ�����)
	bool captureOneFrame(int id);
	//Ϊ�˱����������⣬ʹ�ö��̵߳ķ�ʽ����(��ʱ���������)
	bool captureOneFrameWithTrigger(int id)
	{
		int index =getCameraIndex(id);
		if(index == -1)
		{
			//todo �׳��쳣
		}

		//�����ȴ��߳�
		g_result1 = 1;
		HANDLE hThrds;
		hThrds = CreateThread(NULL,0,CaptureThread1,(LPVOID)(&index),0,NULL);
		Sleep(1);
		//�������
		CameraTriggerShot(index);
		//�ȴ��߳�ִ�����
		WaitForMultipleObjects(1,&hThrds,TRUE,INFINITE);
		//�ر��̺߳��¼�
		CloseHandle(hThrds);

		if(g_result1 ==0)
		{
			//��������
			memcpy(buffers[index],g_buffer0,mBufferLength);
			return true;
		}
		else
		{
			return false;
		}
	}
	//���β�����������ͷ��ͼ��
	bool captureOneFrameAll();
	//����ͬʱ����2��ͼ��
	bool captureTwoFrame(int id1,int id2);
	//���ز���Ļ��棬id��ʾ��ʶ��index��ʾ����
	unsigned char* getBuffer(int id);
	unsigned char* getBufferByIndex(int index);
	cv::Mat getImage(int id);
	cv::Mat getImageByIndex(int index);
//����������ͷ����,����Ϊ0
public:
	//�������
	bool captureOneCameraWithTrigger();
	//����ģʽ
	bool captureOneCameraWithoutTrigger();
//˫��������ͷͬ������
public:
	//������������Ĵ���ģʽ(Ĭ������������ģʽ)
	void setTriggerMode(bool t = true);
	//ͬ����ȡ2��ͼ��(ʹ����������ķ�ʽ)
	bool captureTwoFrameSyncSoftware(int n1,int n2);
	//ͬ����ȡ2��ͼ��(ʹ��Ӳ�������ķ�ʽ,��Ҫ�ⲿ����������������Ƶĵȴ�)
	bool captureTwoFrameSyncHardware(int n1,int n2);
	//ͬ����ȡ2��ͼ����������������ڲ���Ӳ��ͬ����
	bool captureTwoFrameSyncSoftControl(int n1,int n2)
	{
		int index1 =getCameraIndex(n1);
		if(index1 == -1)
		{
			//todo �׳��쳣
		}
		int index2 =getCameraIndex(n2);
		if(index2 == -1)
		{
			//todo �׳��쳣
		}

		//����2���ȴ��߳�
		g_result1 = 1;
		g_result2 = 1;
		HANDLE hThrds[2];
		hThrds[0] = CreateThread(NULL,0,CaptureThread1,(LPVOID)(&index1),0,NULL);
		hThrds[1] = CreateThread(NULL,0,CaptureThread2,(LPVOID)(&index2),0,NULL);
		Sleep(1);
		//�������
		CameraTriggerShot(index1);
		//�ȴ��߳�ִ�����
		WaitForMultipleObjects(2,hThrds,TRUE,INFINITE);
		//�ر��̺߳��¼�
		CloseHandle(hThrds[0]);
		CloseHandle(hThrds[1]);
		if(g_result1 ==0 && g_result2==0 )
		{
			//��������
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
			//todo �׳��쳣
		}
		int index2 =getCameraIndex(n2);
		if(index2 == -1)
		{
			//todo �׳��쳣
		}

		//����2���ȴ��߳�
		g_result1 = 1;
		g_result2 = 1;
		HANDLE hThrds[2];
int t1 = gTimer.getMicroseconds();
		hThrds[0] = CreateThread(NULL,0,CaptureThread1,(LPVOID)(&index1),0,NULL);
		hThrds[1] = CreateThread(NULL,0,CaptureThread2,(LPVOID)(&index2),0,NULL);
int t2= gTimer.getMicroseconds();
		Sleep(1);
int t3 = gTimer.getMicroseconds();
		//�������
		CameraTriggerShot(index1);
int t4 = gTimer.getMicroseconds();
		//�ȴ��߳�ִ�����
		WaitForMultipleObjects(2,hThrds,TRUE,INFINITE);
int t5 = gTimer.getMicroseconds();
		//�ر��̺߳��¼�
		CloseHandle(hThrds[0]);
		CloseHandle(hThrds[1]);
int t6 = gTimer.getMicroseconds();
		if(g_result1 ==0 && g_result2==0 )
		{
			//��������
			memcpy(buffers[index1],g_buffer0,mBufferLength);
			memcpy(buffers[index2],g_buffer1,mBufferLength);
int t7 = gTimer.getMicroseconds();
//��11��
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
	//��ʼ�����е�����������������ź�usb����֮��Ĺ�ϵ
	std::vector<CameraInfos> iniCameraInfos(std::string config="");
private:
	std::vector<CameraInfos> mCamInfo;	//�����������Ϣ(����Ϊ������)
	std::vector<unsigned char*> buffers;//���еĻ���(����������)
	int mCamNum;						//���õ����������
	int mBufferLength;					//�����С
	int mWidth;							//���
	int mHeight;						//�߶�
};
#endif