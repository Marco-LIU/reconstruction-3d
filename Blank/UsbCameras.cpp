#include "UsbCameras.h"

int UsbCameras::g_result1 = 1;
int UsbCameras::g_result2 = 1;
unsigned char* UsbCameras::g_buffer0 = NULL;
unsigned char* UsbCameras::g_buffer1 = NULL;

//��Щȫ�ֱ��������ڲ��Ժ���ִ��ʱ��
Timer UsbCameras::gTimer;
std::vector<int> UsbCameras::gAllTimes;;
int UsbCameras::gt11,UsbCameras::gt12,UsbCameras::gt21,UsbCameras::gt22;

UsbCameras::UsbCameras()
{
  //��ʼ�������豸
  mCamInfo = iniCameraInfos();

  //����֡����
  buffers.resize(mCamNum);
  for(int i=0;i<mCamNum;i++)
  {
    buffers[i] = new unsigned char[mBufferLength];
    memset(buffers[i],0,mBufferLength);
  }

  //��ʼ��ȫ�ֱ���
  g_result1 = 1;
  g_result2 = 1;
  g_buffer0 = new unsigned char[mBufferLength];
  g_buffer1 = new unsigned char[mBufferLength];
}
UsbCameras::UsbCameras(std::string config)
{
  //��ʼ�������豸
  mCamInfo = iniCameraInfos(config);

  //����֡����
  buffers.resize(mCamNum);
  for(int i=0;i<mCamNum;i++)
  {
    buffers[i] = new unsigned char[mBufferLength];
    memset(buffers[i],0,mBufferLength);
  }

  //��ʼ��ȫ�ֱ���
  g_result1 = 1;
  g_result2 = 1;
  g_buffer0 = new unsigned char[mBufferLength];
  g_buffer1 = new unsigned char[mBufferLength];
}
UsbCameras::~UsbCameras()
{
  for(int i=0;i<mCamNum;i++)
  {
    stopCamera(i);
  }

  for(int i=0;i<mCamNum;i++)
  {
    delete [] buffers[i];
  }

  delete [] g_buffer0;
  delete [] g_buffer1;
}

//��ʼ�����е������
std::vector<CameraInfos> UsbCameras::iniCameraInfos(std::string config)
{
  //����������
  int count = 0;
  CameraGetCount(&count);
  mCamNum = count;

  //��ʼ��ΪĬ��ֵ
  std::vector<CameraInfos> g_allCameras(count);

  //��ʼ����������ͷ
  for(int i=0;i<count;i++)
  {
    //��ʼ�����
    int result = CameraInit(i);
    if(result!=API_OK)
    {
      //todo ������
      std::cout<<"�����"<<i<<"��ʼ������"<<std::endl;
    }

    //����Ϊ����ģʽ
    CameraSetHighspeed(i,true);

    //����Ϊ�ߵ�ƽ����
    CameraSetTriggerPolarity(i,true);


    //taokelu@gmail.com: �ֱ������÷ŵ������ļ���
    /*
    //����Ϊ���ֱ��ʣ�1280x1024
    int width,height;
    CameraGetResolutionMax(i,&width,&height);
    result = CameraSetResolution(i,0,&width,&height);
    mWidth = width;
    mHeight = height;
    */

    //��������32����������
    CameraSetAGC(i,false);
    CameraSetGain(i,32);

    //ѹ�����������Ӽ������Ƶ����ȷ�Χ
    CameraSetGamma(i,1.61);

    //��ʹ���Զ��ع�
    CameraSetAEC(i,false);

    //��ȡ12λ�Ĳ�Ʒ���к�
    char id[12];
    CameraReadSerialNumber(i,id,12);
    
    //��¼��ȫ�ֱ�����
    g_allCameras[i].sn =std::string(id,12);
    g_allCameras[i].id = -1;
    g_allCameras[i].index = i;
  }

  //��������ͷ����
  if(config != "")
  {
    //�������ļ��ж�ȡ����ͷ���к�
    cv::FileStorage fs(config,cv::FileStorage::READ);
    for(cv::FileNodeIterator i =fs["Camera"].begin();i!=fs["Camera"].end();i++)
    {							
      std::string nodeName = (*i).name();
      int id = int(*i);

      //�������е�����ͷ��ƥ��sn������
      for(int j=0;j<g_allCameras.size();j++)
      {
        if(g_allCameras[j].sn == nodeName)
        {
          g_allCameras[j].id = id;
        }
      }
    }
    //following modified by taokelu
    //�������ļ��ж�ȡ�ֱ���
    int resolution_index = (int) fs["Resolution"];
    for(int i=0; i<g_allCameras.size(); i++){
        int width,height;
        CameraGetResolution(i, resolution_index, &width, &height);
        CameraSetResolution(i, resolution_index, &width, &height);
        mWidth = width;
        mHeight = height;
    }
    //modifying END
  
    fs.release();
  }
  
  //��¼֡����ĳ���
  int length = 0;
  CameraGetImageBufferSize(0,&length,CAMERA_IMAGE_RAW8);
  mBufferLength = length;

  return g_allCameras;
}
//����ָ���������������-1��ʾ�����û����ȷ��ʼ��
int UsbCameras::getCameraIndex(int id)
{
  for(int i=0;i<mCamInfo.size();i++)
  {
    if(mCamInfo[i].id == id)
      return mCamInfo[i].index;
  }
  std::cout<<"�Ҳ���id="<<id<<"������ͷ"<<std::endl;
  return -1;
}
//��������ָ���������id
int UsbCameras::getCameraId(int index)
{
  for(int i=0;i<mCamInfo.size();i++)
  {
    if(mCamInfo[i].index == index)
    {
      return mCamInfo[i].id;
    }
  }

  return -1;
}
//��������ָ���������id
std::string UsbCameras::getCameraSn(int index)
{
  for(int i=0;i<mCamInfo.size();i++)
  {
    if(mCamInfo[i].index == index)
    {
      return mCamInfo[i].sn;
    }
  }

  return "dijkstra";
}
//����ϵͳ�п���ʹ�õ�������ĸ���
int UsbCameras::getCameraCount()
{
  return mCamNum;
}
//ֹͣ�����
void UsbCameras::stopCamera(int index)
{
  CameraStop(index);	//ֹͣ���
  CameraFree(index);	//�ͷ����
}

//��д��i���������ͷ������,-1��ʾʧ��
int UsbCameras::getGain(int id)
{
  int index =getCameraIndex(id);
  if(index == -1)
  {
    //todo �׳��쳣
  }
  int gain;
  API_STATUS status = CameraGetGain(index,&gain);
  if(status != API_OK)
  {
    //��ȡʧ��
    return -1;
  }
  return gain;
}
bool UsbCameras::setGain(int id,int gain)
{
  int index =getCameraIndex(id);
  if(index == -1)
  {
    //todo �׳��쳣
  }
  API_STATUS status = CameraSetGain(index,gain);
  if(status != API_OK)
  {
    return false;
  }
  return true;
}
bool UsbCameras::setGainAll(int gain)
{
  for(int i=0;i<mCamNum;i++)
  {
    API_STATUS status = CameraSetGain(i,gain);
    if(status != API_OK)
    {
      //todo �׳��쳣
      return false;
    }
  }
  return true;
}
void UsbCameras::setAutoGain(bool t)
{
  for(int i=0;i<mCamNum;i++)
  {
    CameraSetAGC(i,t);
  }
}

//��д��i���������ͷ���ع�
int UsbCameras::getExposure(int id)
{
  int index =getCameraIndex(id);
  if(index == -1)
  {
    //todo �׳��쳣
  }
  int expo;
  API_STATUS status = CameraGetExposure(index,&expo);
  if(status != API_OK)
  {
    //��ȡʧ��
    return -1;
  }
  return expo;
}
bool UsbCameras::setExposure(int id,int expo)
{
  int index =getCameraIndex(id);
  if(index == -1)
  {
    //todo �׳��쳣
  }
  API_STATUS status = CameraSetExposure(index,expo);
  if(status != API_OK)
  {
    return false;
  }
  return true;
}
bool UsbCameras::setExposureAll(int expo)
{
  for(int i=0;i<mCamNum;i++)
  {
    API_STATUS status = CameraSetExposure(i,expo);
    if(status != API_OK)
    {
      //todo �׳��쳣
      return false;
    }
  }
  return true;
}
void UsbCameras::setAutoExposure(bool t)
{
  for(int i=0;i<mCamNum;i++)
  {
    CameraSetAEC(i,t);
  }
}

//ֹͣ����� todo ���Կ�����
void UsbCameras::stopAllCamera()
{
  for(int i=0;i<mCamNum;i++)
  {
    CameraStop(i);
  }
}
void UsbCameras::stopOneCamera(int id)
{
  int index = getCameraIndex(id);
  if(index !=-1)
    CameraStop(index);
}
//���������
void UsbCameras::startAllCamera()
{
  for(int i=0;i<mCamNum;i++)
  {
    CameraPlay(i,NULL,NULL);
  }
}
void UsbCameras::startOneCamera(int id)
{
  int index = getCameraIndex(id);
  if(index !=-1)
    CameraPlay(index,NULL,NULL);
}

/*******************************************************������*******************************************/
//��ȡ�ض�����ͷ��һ֡ͼ��
bool UsbCameras::captureOneFrame(int id)
{
  int index =getCameraIndex(id);
  if(index == -1)
  {
    //todo �׳��쳣
  }

  //��ȡ���ݣ���¼����Ӧ�Ļ�����
  int result = CameraQueryImage(index,buffers[index],&mBufferLength,CAMERA_IMAGE_RAW8);
  if(result ==0)
  {
    return true;
  }
  else
  {
    return false;
  }
}
//��ȡ��������ͷ��ͼ��
bool UsbCameras::captureOneFrameAll()
{
  bool ret = true;
  for(int i=0;i<mCamNum;i++)
  {
    if(mCamInfo[i].index !=-1)
    {
      bool rlt = captureOneFrame(i);
      if(rlt == false)
      {
        ret = rlt;
      }
    }
  }
  return ret;
}
//����2��ͼ��
bool UsbCameras::captureTwoFrame(int id1,int id2)
{
  int index1 =getCameraIndex(id1);
  if(index1 == -1)
  {
    //todo �׳��쳣
  }
  int index2 =getCameraIndex(id2);
  if(index2 == -1)
  {
    //todo �׳��쳣
  }
  //��ȡ���ݣ���¼����Ӧ�Ļ�����
  int result1 = CameraQueryImage(index1,buffers[index1],&mBufferLength,CAMERA_IMAGE_RAW8);
  int result2 = CameraQueryImage(index2,buffers[index2],&mBufferLength,CAMERA_IMAGE_RAW8);
  if(result1 ==0 && result2==0)
  {
    return true;
  }
  else
  {
    return false;
  }
}
//���ز���Ļ��棬����Ϊid
unsigned char* UsbCameras::getBuffer(int id)
{
  int index = getCameraIndex(id);
  return buffers[index];
}
unsigned char* UsbCameras::getBufferByIndex(int index)
{
  return buffers[index];
}
cv::Mat UsbCameras::getImage(int id)
{
  int index = getCameraIndex(id);
  return cv::Mat(mHeight,mWidth,CV_8UC1,buffers[index]);
}
cv::Mat UsbCameras::getImageByIndex(int index)
{
  return cv::Mat(mHeight,mWidth,CV_8UC1,buffers[index]);
}

//����������,����汾����Ҫ��CAMERA_IMAGE_RAW8��ΪCAMERA_IMAGE_GRAY8
bool UsbCameras::captureOneCameraWithTrigger()
{
  //��ȡ���ݣ���¼����Ӧ�Ļ�����
  int result = CameraQueryImage(0,buffers[0],&mBufferLength,CAMERA_IMAGE_GRAY8|CAMERA_IMAGE_TRIG);
  if(result ==0)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool UsbCameras::captureOneCameraWithoutTrigger()
{
  //��ȡ���ݣ���¼����Ӧ�Ļ�����
  int result = CameraQueryImage(0,buffers[0],&mBufferLength,CAMERA_IMAGE_RAW8);
  if(result ==0)
  {
    return true;
  }
  else
  {
    return false;
  }
}

/*******************************************************ͬ������*******************************************/
void UsbCameras::setTriggerMode(bool t)
{
  if(t)
  {
    for(int i=0;i<mCamNum;i++)
    {
      CameraSetSnapMode(i,CAMERA_SNAP_TRIGGER);
    }
  }
  else
  {
    for(int i=0;i<mCamNum;i++)
    {
      CameraSetSnapMode(i,CAMERA_SNAP_CONTINUATION);
    }
  }
}

bool UsbCameras::captureTwoFrameSyncSoftware(int n1,int n2)
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
  Sleep(3);

  //�������
  CameraTriggerShot(index1);
  CameraTriggerShot(index2);

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

bool UsbCameras::captureTwoFrameSyncHardware(int n1,int n2)
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
  Sleep(3);

  /*
  //�ⲿ����
  //���ڵ�Ƭ������
  CnComm mc;
  mc.Open(5);
  unsigned char open = 255;
  mc.Write( (LPCVOID)(&open),1);
  mc.Flush();
  mc.Close();
  */
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