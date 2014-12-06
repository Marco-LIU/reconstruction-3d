#pragma once
#include "base/containers/hash_tables.h"
#include "base/memory/ref_counted_memory.h"
#include "base/memory/scoped_ptr.h"

class CameraPump;

struct CameraFrame;

typedef base::hash_map<int, CameraFrame> CameraFrames;

class CameraGroupPumpDelegate {
public:
  virtual ~CameraGroupPumpDelegate() {}

  virtual void OnFrame(scoped_ptr<CameraFrames> frames) = 0;
};
