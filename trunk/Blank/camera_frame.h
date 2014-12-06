#pragma once
#include "base/memory/ref_counted_memory.h"
#include "base/time/time.h"
#include "base/containers/hash_tables.h"

#include "QtGui/qimage.h"

struct CameraFrame {
  unsigned int frame_seq;
  unsigned int width;
  unsigned int height;
  short bpp;
  base::Time time_stamp;
  scoped_refptr<base::RefCountedBytes> data;

  mutable QImage cached_image;

  QImage ToQImage() const;
};

typedef base::hash_map<int, CameraFrame> CameraFrames;