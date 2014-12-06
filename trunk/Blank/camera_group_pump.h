#pragma once
#include <vector>

#include "base/memory/linked_ptr.h"
#include "base/memory/ref_counted.h"

class UsbCameraGroup;
class CameraGroupPumpDelegate;

class CameraGroupPump {
public:
  CameraGroupPump(UsbCameraGroup* camera_group,
                  CameraGroupPumpDelegate* delegate);

  ~CameraGroupPump();

  bool IsPumping() const;

  bool StartPumping();
  void StopPumping();

private:
  class Context;
  scoped_refptr<Context> context_;
};