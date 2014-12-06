#pragma once
#include "base/memory/ref_counted_memory.h"

struct CameraFrame;

// ����ӿڣ��ӿں����ں�̨�߳��е���
// ʵ��ʱע��ͬ��
class CameraPumpDelegate {
public:
  virtual ~CameraPumpDelegate() {}

  // ��ʼ¼��һ֡
  virtual bool BeforeFrame(int id) = 0;

  // ��������ڴ�
  virtual scoped_refptr<base::RefCountedBytes> WillRead(int id,
                                                        unsigned int len) = 0;

  // ֪ͨץȡ��һ֡��frame_seq��ʾ֡��ţ�dataΪ֡���ݣ��ǿձ�ʾ��Ч��
  virtual void OnFrame(int id,
                       const CameraFrame& frame) = 0;

  // ¼�����һ֡
  virtual bool AfterFrame(int id) = 0;

  // ¼�����
  virtual void OnDone(int id, unsigned int frames) = 0;
};