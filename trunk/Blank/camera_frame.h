#pragma once

#include "base/memory/ref_counted_memory.h"

#include "QtGui/qimage.h"

struct CameraFrame {
  unsigned int frame_seq;
  unsigned int width;
  unsigned int height;
  short bpp;
  scoped_refptr<base::RefCountedBytes> data;

  QImage ToQImage() const;
};