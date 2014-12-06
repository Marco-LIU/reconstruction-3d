#pragma once
#include "base/memory/ref_counted_memory.h"
#include "base/memory/scoped_ptr.h"

#include "camera_frame.h"

class CameraPump;

class CameraGroupPumpDelegate {
public:
  virtual ~CameraGroupPumpDelegate() {}

  virtual void OnFrame(scoped_ptr<CameraFrames> frames) = 0;
};
