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

  bool Init(const std::string& config);

  bool StartAll();

  void StopAll();

  unsigned int camera_count() const;

  scoped_refptr<UsbCamera> GetCamera(int id) const;

  const CameraMap& cameras() const { return cameras_; }

  void SoftTriggerAll();

  // 同步抓取一帧
  bool CaptureFrame(bool use_trigger, const BufferMap* buffers = NULL);

  const unsigned char* camera_buffer(int id) const;

  virtual void OnFrame(scoped_ptr<CameraFrames> frames) OVERRIDE;

  void GetFrames(CameraFrames& frame_map);

  typedef base::Callback<void(CameraFrames&)> FrameCallback;

  void SetFrameCallback(FrameCallback callback);

protected:
  CameraMap cameras_;

  BufferMap buffers_;

  scoped_ptr<CameraGroupPump> group_pump_;

  scoped_refptr<base::MessageLoopProxy> born_loop_;

  CameraFrames cached_frames_;

  FrameCallback frame_callback_;
};