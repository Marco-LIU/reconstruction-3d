#pragma once
#include "base/memory/ref_counted_memory.h"

class UsbCamera;
class CameraPumpDelegate;

class CameraPump {
public:
  static scoped_refptr<base::RefCountedBytes> NewBuffer(unsigned int len);
public:
  CameraPump(scoped_refptr<UsbCamera> camera,
             CameraPumpDelegate* delegate);
  ~CameraPump();

  bool IsPumping() const;

  bool StartPumping();
  void StopPumping();

private:
  class Context;
  scoped_refptr<Context> context_;
};