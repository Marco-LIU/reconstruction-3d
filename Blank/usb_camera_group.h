#pragma once
#include <string>
#include <map>

#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop_proxy.h"

#include "camera_group_pump_delegate.h"

class UsbCamera;
class CameraGroupPump;

// 摄像头组
// 必须在UI线程中创建
class UsbCameraGroup
    : public CameraGroupPumpDelegate
    , public base::RefCountedThreadSafe<UsbCameraGroup>
{
public:
  UsbCameraGroup();
  ~UsbCameraGroup();

  typedef std::map<int, unsigned char*> BufferMap;

  // id -> camera
  typedef std::map<int, scoped_refptr<UsbCamera> > CameraMap;

  // 使用配置文件初始化摄像头组
  // 当前合法的配置文件为：para.config或者para.json
  bool Init(const std::string& config);

  bool StartAll();

  void StopAll();

  unsigned int camera_count() const;

  scoped_refptr<UsbCamera> GetCamera(int id) const;

  const CameraMap& cameras() const { return cameras_; }

  void SoftTriggerAll();

  //////////////////////////////////////////////////////////////////////////
  // 老式的获取帧数据接口
  // 同步抓取一帧
  bool CaptureFrame(bool use_trigger, const BufferMap* buffers = NULL);

  const unsigned char* camera_buffer(int id) const;

  //////////////////////////////////////////////////////////////////////////
  // 获取当前缓存的CameraFrames
  void GetFrames(CameraFrames& frame_map);

  typedef base::Callback<void(CameraFrames&)> FrameCallback;

  // 设置帧处理的回调函数，回调函数可以更改传入的CameraFrames，达到过滤效果
  // 回调函数在获取一帧后调用
  // 回调函数返回后，CameraFrames会被缓存
  void SetFrameCallback(FrameCallback callback);

  // 当前帧率
  float FrameRate();

protected:
  // impl CameraGroupPumpDelegate
  virtual void OnFrame(scoped_ptr<CameraFrames> frames) OVERRIDE;

protected:
  CameraMap cameras_;

  BufferMap buffers_;

  scoped_ptr<CameraGroupPump> group_pump_;

  // 创建此对象的消息循环
  scoped_refptr<base::MessageLoopProxy> born_loop_;

  CameraFrames cached_frames_;

  FrameCallback frame_callback_;
};