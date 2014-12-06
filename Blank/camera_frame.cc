#include "camera_frame.h"
#include "qt_utils.h"

QImage CameraFrame::ToQImage() const {
  if (!cached_image.isNull())
    return cached_image;

  cached_image = FromRawGray(data, width, height);
  return cached_image;
}