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

// ����ͷ��
// ������UI�߳��д���
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

  // ʹ�������ļ���ʼ������ͷ��
  // ��ǰ�Ϸ��������ļ�Ϊ��para.config����para.json
  bool Init(const std::string& config);

  bool StartAll();

  void StopAll();

  unsigned int camera_count() const;

  scoped_refptr<UsbCamera> GetCamera(int id) const;

  const CameraMap& cameras() const { return cameras_; }

  void SoftTriggerAll();

  //////////////////////////////////////////////////////////////////////////
  // ��ʽ�Ļ�ȡ֡���ݽӿ�
  // ͬ��ץȡһ֡
  bool CaptureFrame(bool use_trigger, const BufferMap* buffers = NULL);

  const unsigned char* camera_buffer(int id) const;

  //////////////////////////////////////////////////////////////////////////
  // ��ȡ��ǰ�����CameraFrames
  void GetFrames(CameraFrames& frame_map);

  typedef base::Callback<void(CameraFrames&)> FrameCallback;

  // ����֡����Ļص��������ص��������Ը��Ĵ����CameraFrames���ﵽ����Ч��
  // �ص������ڻ�ȡһ֡�����
  // �ص��������غ�CameraFrames�ᱻ����
  void SetFrameCallback(FrameCallback callback);

  // ��ǰ֡��
  float FrameRate();

protected:
  // impl CameraGroupPumpDelegate
  virtual void OnFrame(scoped_ptr<CameraFrames> frames) OVERRIDE;

protected:
  CameraMap cameras_;

  BufferMap buffers_;

  scoped_ptr<CameraGroupPump> group_pump_;

  // �����˶������Ϣѭ��
  scoped_refptr<base::MessageLoopProxy> born_loop_;

  CameraFrames cached_frames_;

  FrameCallback frame_callback_;
};