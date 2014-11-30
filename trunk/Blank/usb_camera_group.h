#pragma once
#include <string>
#include <map>

class UsbCamera;

// ����ӿڣ��ӿں����ں�̨�߳��е���
// ʵ��ʱע��ͬ��
class CameraGroupRecordDelegate {
public:
  virtual ~CameraGroupRecordDelegate() {}

  virtual std::string GetSavingDir(unsigned int cam_id) = 0;

  // ֪ͨץȡ��һ֡��index��ʾ֡���
  // ����Ϊfalseʱ���Դ�֡
  virtual bool OnFrame(unsigned int cam_id,
                       unsigned int frame_seq,
                       const unsigned char* data,
                       unsigned int len) = 0;
};

class UsbCameraGroup
{
public:
  UsbCameraGroup();
  ~UsbCameraGroup();

  typedef std::map<int, unsigned char*> BufferMap;

  bool Init(const std::string& config);

  bool StartAll();

  void StopAll();

  unsigned int camera_count() const;

  UsbCamera* GetCamera(int id) const;

  // ͬ��ץȡһ֡
  bool CaptureFrame(bool use_trigger, const BufferMap* buffers = NULL);

  const unsigned char* camera_buffer(int id) const;

  bool StartRecord(CameraGroupRecordDelegate* delegate);
  void StopRecord();

protected:
  typedef std::map<int, UsbCamera*> CameraMap;

  CameraMap cameras_;

  BufferMap buffers_;

  class RecordContext;
  RecordContext* record_context_;
};