#include "camera_frame.h"
#include "qt_utils.h"

QImage CameraFrame::ToQImage() const {
  return FromRawGray(data, width, height);
}