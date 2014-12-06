#include "frame_record.h"

QImage FrameRecord::ToQImage() const {
  if (!cached_image.isNull()) return cached_image;
  cached_image.load(QString::fromWCharArray(image_path.value().c_str()));
  return cached_image;
}

void FrameRecord::Purge() {
  cached_image.swap(QImage());
}