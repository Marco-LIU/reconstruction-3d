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
  void SetId(int id);
  //��������ָ�����������sn
  std::string sn() const { return sn_; }
  //����ϵͳ�п���ʹ�õ�������ĸ���

  //ֹͣ���ͷ��������Դ
  void Stop();
  //���������
  bool Start();

  unsigned int width() const { return width_; }

  unsigned int height() const { return height_; }

  unsigned int buffer_length() const { return buffer_length_; }

  const unsigned char* buffer() const { return buffer_; }

  bool SoftTrigger();

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
  virtual void OnGainChanged(int id, int gain) OVERRIDE;
  virtual void OnExposureChanged(int id, int expo) OVERRIDE;

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
private:
  friend class base::RefCountedThreadSafe<UsbCamera>;
};