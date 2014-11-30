#pragma once
#include <Windows.h>
#include <string>

// ����ӿڣ��ӿں����ں�̨�߳��е���
// ʵ��ʱע��ͬ��
class CameraRecordDelegate {
public:
  virtual ~CameraRecordDelegate() {}

  virtual std::string GetSavingDir() = 0;

  // ֪ͨץȡ��һ֡��index��ʾ֡���
  // ����Ϊfalseʱ���Դ�֡
  virtual bool OnFrame(unsigned int frame_seq,
                       const unsigned char* data,
                       unsigned int len) = 0;
};

class UsbCamera
{
public:
  UsbCamera(int index);
  UsbCamera(int index, int id);
  ~UsbCamera();
public:
  bool Init();

  //��д��i���������ͷ������,-1��ʾʧ��
  int GetGain() const;
  bool SetGain(int gain);
  bool SetAutoGain(bool t = true);

  //��д��i���������ͷ���ع�
  int GetExposure() const;
  bool SetExposure(int expo);
  bool SetAutoExposure(bool t = true);

  //����ָ���������������-1��ʾ�����û����ȷ��ʼ��
  int index() const { return index_; }
  //��������ָ���������id
  int id() const { return id_; }
  void SetId(int id) { id_ = id; }
  //��������ָ�����������sn
  std::string sn() const { return sn_; }
  //����ϵͳ�п���ʹ�õ�������ĸ���

  //ֹͣ���ͷ��������Դ
  void Stop();
  //���������
  bool Start();

  // TODO��δʵ��
  bool StartRecord(CameraRecordDelegate* delegate) { return false; }
  void StopRecord() {}

  unsigned int width() const { return width_; }

  unsigned int height() const { return height_; }

  unsigned int buffer_length() const { return buffer_length_; }

  const unsigned char* buffer() const { return buffer_; }

  // ͬ��ץȡһ֡��������ץȡ��ɺ󷵻�
  // ���첽ץȡ�Ĺ����е��ô˷�����ʧ��
  bool CaptureFrameSync(bool use_trigger, unsigned char* buffer = NULL);

  // �첽ץȡһ֡�������������أ�ͨ������HANDLEȷ���Ƿ����
  // ���첽ץȡ�Ĺ����е��ô˷�����ʧ�ܣ�����NULL
  // TODO��ʵ���첽��ɵĻص�
  HANDLE CaptureFrameAsync(bool use_trigger, unsigned char* buffer = NULL);

  // �첽�����Ƿ�ɹ�
  bool IsAsyncSuccess(HANDLE handle);

  bool async_capturing() const { return async_capturing_ != 0; }

protected:
  unsigned int buffer_length_;    // �����С
  unsigned int width_;            // ���
  unsigned int height_;           // �߶�
  unsigned char* buffer_;         // �ڲ�����

  // camera info
  int id_;
  int index_;
  std::string sn_;

  volatile LONG async_capturing_;

  class RecordContext;
  RecordContext* record_context_;
};