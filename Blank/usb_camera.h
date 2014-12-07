#pragma once
#include <Windows.h>
#include <string>

#include "base/memory/ref_counted.h"

#include "camera_config.h"
#include "paras.h"

class UsbCamera
    : public ParaObserver
    , public base::RefCountedThreadSafe<UsbCamera>
{
public:
  UsbCamera(int index);
  UsbCamera(int index, int id);
  ~UsbCamera();
public:
  bool Init(int resolution_index = 0);

  void SetConfig(const CameraConfig& cfg);

  //读写第i个编号摄像头的增益,-1表示失败
  int GetGain() const;
  bool SetGain(int gain);
  bool SetAutoGain(bool t = true);

  //读写第i个编号摄像头的曝光
  int GetExposure() const;
  bool SetExposure(int expo);
  bool SetAutoExposure(bool t = true);

  //返回指定摄像机的索引，-1表示摄像机没有正确初始化
  int index() const { return index_; }
  //返回索引指定的摄像机id
  int id() const { return id_; }
  void SetId(int id);
  //返回索引指定的摄像机的sn
  std::string sn() const { return sn_; }
  //返回系统中可以使用的摄像机的个数

  //停止并释放摄像机资源
  void Stop();
  //启动摄像机
  bool Start();

  unsigned int width() const { return width_; }

  unsigned int height() const { return height_; }

  unsigned int buffer_length() const { return buffer_length_; }

  const unsigned char* buffer() const { return buffer_; }

  bool SoftTrigger();

  // 同步抓取一帧，函数在抓取完成后返回
  // 在异步抓取的过程中调用此方法会失败
  bool CaptureFrameSync(bool use_trigger, unsigned char* buffer = NULL);

  // 异步抓取一帧，函数立即返回，通过返回HANDLE确定是否完成
  // 在异步抓取的过程中调用此方法会失败，返回NULL
  // TODO：实现异步完成的回调
  HANDLE CaptureFrameAsync(bool use_trigger, unsigned char* buffer = NULL);

  // 异步操作是否成功
  bool IsAsyncSuccess(HANDLE handle);

  bool async_capturing() const { return async_capturing_ != 0; }

protected:
  virtual void OnGainChanged(int id, int gain) OVERRIDE;
  virtual void OnExposureChanged(int id, int expo) OVERRIDE;

protected:
  unsigned int buffer_length_;    // 缓存大小
  unsigned int width_;            // 宽度
  unsigned int height_;           // 高度
  unsigned char* buffer_;         // 内部缓冲

  // camera info
  int id_;
  int index_;
  std::string sn_;

  volatile LONG async_capturing_;
private:
  friend class base::RefCountedThreadSafe<UsbCamera>;
};