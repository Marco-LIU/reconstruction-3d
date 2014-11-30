#pragma once
#include <string>
#include <map>

class UsbCamera;

class CameraGroupRecordDelegate {
public:
  virtual ~CameraGroupRecordDelegate() {}

  virtual std::string GetSavingDir() = 0;

  // 通知抓取了一帧，index表示帧序号
  // 返回为false时忽略此帧
  virtual bool OnFrame(unsigned int cam_id,
                       unsigned int frame_seq,
                       const unsigned char* data,
                       unsigned int len) = 0;
};

class UsbCameraGroup
{
public:
  UsbCameraGroup();
  UsbCameraGroup(const std::string& config_content);
  ~UsbCameraGroup();

  bool Init();

  bool StartAll();

  bool StopAll();

  unsigned int camera_count() const;

  UsbCamera* GetCamera(int id) const;

  // 同步抓取一帧
  bool CaptureFrame(bool use_trigger, unsigned char** buffers = NULL);

  const unsigned char* camera_buffer(int id) const;

protected:
  typedef std::map<int, UsbCamera*> CameraMap;

  CameraMap cameras_;

  unsigned char** buffers_;

  class RecordContext;
  RecordContext* record_context_;
};