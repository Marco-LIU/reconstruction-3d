#include "UsbCameras.h"

int UsbCameras::g_result1 = 1;
int UsbCameras::g_result2 = 1;
unsigned char* UsbCameras::g_buffer0 = NULL;
unsigned char* UsbCameras::g_buffer1 = NULL;

//这些全局变量，用于测试函数执行时间
Timer UsbCameras::gTimer;
std::vector<int> UsbCameras::gAllTimes;;
int UsbCameras::gt11,UsbCameras::gt12,UsbCameras::gt21,UsbCameras::gt22;

UsbCameras::UsbCameras()
{
  //初始化所有设备
  mCamInfo = iniCameraInfos();

  //分配帧缓存
  buffers.resize(mCamNum);
  for(int i=0;i<mCamNum;i++)
  {
    buffers[i] = new unsigned char[mBufferLength];
    memset(buffers[i],0,mBufferLength);
  }

  //初始化全局变量
  g_result1 = 1;
  g_result2 = 1;
  g_buffer0 = new unsigned char[mBufferLength];
  g_buffer1 = new unsigned char[mBufferLength];
}
UsbCameras::UsbCameras(std::string config)
{
  //初始化所有设备
  mCamInfo = iniCameraInfos(config);

  //分配帧缓存
  buffers.resize(mCamNum);
  for(int i=0;i<mCamNum;i++)
  {
    buffers[i] = new unsigned char[mBufferLength];
    memset(buffers[i],0,mBufferLength);
  }

  //初始化全局变量
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

//初始化所有的摄像机
std::vector<CameraInfos> UsbCameras::iniCameraInfos(std::string config)
{
  //获得相机个数
  int count = 0;
  CameraGetCount(&count);
  mCamNum = count;

  //初始化为默认值
  std::vector<CameraInfos> g_allCameras(count);

  //初始化所有摄像头
  for(int i=0;i<count;i++)
  {
    //初始化相机
    int result = CameraInit(i);
    if(result!=API_OK)
    {
      //todo 错误处理
      std::cout<<"摄像机"<<i<<"初始化错误"<<std::endl;
    }

    //设置为高速模式
    CameraSetHighspeed(i,true);

    //设置为高电平触发
    CameraSetTriggerPolarity(i,true);


    //taokelu@gmail.com: 分辨率设置放到配置文件中
    /*
    //设置为最大分辨率，1280x1024
    int width,height;
    CameraGetResolutionMax(i,&width,&height);
    result = CameraSetResolution(i,0,&width,&height);
    mWidth = width;
    mHeight = height;
    */

    //设置增益32，减少噪声
    CameraSetAGC(i,false);
    CameraSetGain(i,32);

    //压缩暗部，增加激光条纹的亮度范围
    CameraSetGamma(i,1.61);

    //不使用自动曝光
    CameraSetAEC(i,false);

    //获取12位的产品序列号
    char id[12];
    CameraReadSerialNumber(i,id,12);
    
    //记录到全局变量中
    g_allCameras[i].sn =std::string(id,12);
    g_allCameras[i].id = -1;
    g_allCameras[i].index = i;
  }

  //设置摄像头索引
  if(config != "")
  {
    //从配置文件中读取摄像头序列号
    cv::FileStorage fs(config,cv::FileStorage::READ);
    for(cv::FileNodeIterator i =fs["Camera"].begin();i!=fs["Camera"].end();i++)
    {							
      std::string nodeName = (*i).name();
      int id = int(*i);

      //遍历所有的摄像头，匹配sn和索引
      for(int j=0;j<g_allCameras.size();j++)
      {
        if(g_allCameras[j].sn == nodeName)
        {
          g_allCameras[j].id = id;
        }
      }
    }
    //following modified by taokelu
    //从配置文件中读取分辨率
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
  
  //记录帧缓存的长度
  int length = 0;
  CameraGetImageBufferSize(0,&length,CAMERA_IMAGE_RAW8);
  mBufferLength = length;

  return g_allCameras;
}
//返回指定摄像机的索引，-1表示摄像机没有正确初始化
int UsbCameras::getCameraIndex(int id)
{
  for(int i=0;i<mCamInfo.size();i++)
  {
    if(mCamInfo[i].id == id)
      return mCamInfo[i].index;
  }
  std::cout<<"找不到id="<<id<<"的摄像头"<<std::endl;
  return -1;
}
//返回索引指定的摄像机id
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
//返回索引指定的摄像机id
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
//返回系统中可以使用的摄像机的个数
int UsbCameras::getCameraCount()
{
  return mCamNum;
}
//停止摄像机
void UsbCameras::stopCamera(int index)
{
  CameraStop(index);	//停止相机
  CameraFree(index);	//释放相机
}

//读写第i个编号摄像头的增益,-1表示失败
int UsbCameras::getGain(int id)
{
  int index =getCameraIndex(id);
  if(index == -1)
  {
    //todo 抛出异常
  }
  int gain;
  API_STATUS status = CameraGetGain(index,&gain);
  if(status != API_OK)
  {
    //获取失败
    return -1;
  }
  return gain;
}
bool UsbCameras::setGain(int id,int gain)
{
  int index =getCameraIndex(id);
  if(index == -1)
  {
    //todo 抛出异常
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
      //todo 抛出异常
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

//读写第i个编号摄像头的曝光
int UsbCameras::getExposure(int id)
{
  int index =getCameraIndex(id);
  if(index == -1)
  {
    //todo 抛出异常
  }
  int expo;
  API_STATUS status = CameraGetExposure(index,&expo);
  if(status != API_OK)
  {
    //获取失败
    return -1;
  }
  return expo;
}
bool UsbCameras::setExposure(int id,int expo)
{
  int index =getCameraIndex(id);
  if(index == -1)
  {
    //todo 抛出异常
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
      //todo 抛出异常
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

//停止摄像机 todo 测试可用性
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
//启动摄像机
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

/*******************************************************捕获函数*******************************************/
//读取特定摄像头的一帧图像
bool UsbCameras::captureOneFrame(int id)
{
  int index =getCameraIndex(id);
  if(index == -1)
  {
    //todo 抛出异常
  }

  //读取数据，记录到对应的缓存中
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
//读取所有摄像头的图像
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
//捕获2张图像
bool UsbCameras::captureTwoFrame(int id1,int id2)
{
  int index1 =getCameraIndex(id1);
  if(index1 == -1)
  {
    //todo 抛出异常
  }
  int index2 =getCameraIndex(id2);
  if(index2 == -1)
  {
    //todo 抛出异常
  }
  //读取数据，记录到对应的缓存中
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
//返回捕获的缓存，参数为id
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

//测试有问题,这个版本中需要把CAMERA_IMAGE_RAW8改为CAMERA_IMAGE_GRAY8
bool UsbCameras::captureOneCameraWithTrigger()
{
  //读取数据，记录到对应的缓存中
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
  //读取数据，记录到对应的缓存中
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

/*******************************************************同步捕获*******************************************/
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
  Sleep(3);

  //软件触发
  CameraTriggerShot(index1);
  CameraTriggerShot(index2);

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

bool UsbCameras::captureTwoFrameSyncHardware(int n1,int n2)
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
  Sleep(3);

  /*
  //外部触发
  //串口单片机触发
  CnComm mc;
  mc.Open(5);
  unsigned char open = 255;
  mc.Write( (LPCVOID)(&open),1);
  mc.Flush();
  mc.Close();
  */
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