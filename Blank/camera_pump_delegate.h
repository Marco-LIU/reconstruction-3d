#pragma once
#include "base/memory/ref_counted_memory.h"

struct CameraFrame;

// 代理接口，接口函数在后台线程中调用
// 实现时注意同步
class CameraPumpDelegate {
public:
  virtual ~CameraPumpDelegate() {}

  // 开始录制一帧
  virtual bool BeforeFrame(int id) = 0;

  // 请求分配内存
  virtual scoped_refptr<base::RefCountedBytes> WillRead(int id,
                                                        unsigned int len) = 0;

  // 通知抓取了一帧，frame_seq表示帧序号，data为帧数据（非空表示有效）
  virtual void OnFrame(int id,
                       const CameraFrame& frame) = 0;

  // 录制完成一帧
  virtual bool AfterFrame(int id) = 0;

  // 录制完成
  virtual void OnDone(int id, unsigned int frames) = 0;
};