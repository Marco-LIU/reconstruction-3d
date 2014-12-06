#include "usb_camera.h"
#include "JHCap.h"

#include <iostream>

UsbCamera::UsbCamera(int index)
  : index_(index)
  , id_(0)
  , buffer_(NULL)
  , async_capturing_(0) {}

UsbCamera::UsbCamera(int index, int id)
    : index_(index)
    , id_(id)
    , buffer_(NULL)
    , async_capturing_(0) {
}

UsbCamera::~UsbCamera() {
  if (buffer_) delete[] buffer_;
  Stop();
}

bool UsbCamera::Init() {
  int result = CameraInit(index_);
  if (API_OK != result) {
    // TODO: report error
    return false;
  }

  //设置为高速模式
  CameraSetHighspeed(index_, true);

  //设置为高电平触发
  CameraSetTriggerPolarity(index_, true);

  //设置为最大分辨率，1280x1024
  int width = 640, height = 480;
  //CameraGetResolutionMax(index_, &width, &height);
  result = CameraSetResolution(index_, 3, &width, &height);
  width_ = width;
  height_ = height;

  //设置增益32，减少噪声
  CameraSetAGC(index_, false);
  CameraSetGain(index_, 32);

  //压缩暗部，增加激光条纹的亮度范围
  CameraSetGamma(index_, 1.61);

  //不使用自动曝光
  CameraSetAEC(index_, false);

  CameraSetSnapMode(index_, CAMERA_SNAP_TRIGGER);

  //获取12位的产品序列号
  char id[13];
  CameraReadSerialNumber(index_, id, 12);
  id[12] = '\0';
  sn_ = id;

  int length = 0;
  CameraGetImageBufferSize(0, &length, CAMERA_IMAGE_GRAY8);
  buffer_length_ = length;

  buffer_ = new unsigned char[buffer_length_];

  return true;
}
int UsbCamera::GetGain() const {
  int gain;
  API_STATUS status = CameraGetGain(index_, &gain);
  if (status != API_OK) {
    //获取失败
    return -1;
  }
  return gain;
}

bool UsbCamera::SetGain(int gain) {
  return API_OK == CameraSetGain(index_, gain);
}

bool UsbCamera::SetAutoGain(bool t /* = true */) {
  return API_OK == CameraSetAGC(index_, t);
}

int UsbCamera::GetExposure() const {
  int expo;
  API_STATUS status = CameraGetExposure(index_, &expo);
  if (status != API_OK) {
    //获取失败
    return -1;
  }
  return expo;
}

bool UsbCamera::SetExposure(int expo) {
  return API_OK == CameraSetExposure(index_, expo);
}

bool UsbCamera::SetAutoExposure(bool t /* = true */) {
  return API_OK == CameraSetAEC(index_, t);
}

bool UsbCamera::Start() {
  return API_OK == CameraPlay(index_, NULL, NULL);
}

void UsbCamera::Stop() {
  CameraStop(index_);
}

bool UsbCamera::SoftTrigger() {
  return API_OK == CameraTriggerShot(index_);
}

bool UsbCamera::CaptureFrameSync(bool use_trigger,
                                 unsigned char* buffer /* = NULL */) {
  if (async_capturing_ > 0) return false;
  if (!buffer_ || !buffer) return false;
  int opt = CAMERA_IMAGE_RAW8;
  if (use_trigger) {
    opt = CAMERA_IMAGE_GRAY8 | CAMERA_IMAGE_TRIG;
  }
  int len = buffer_length_;
  if (!buffer)
    buffer = buffer_;
  API_STATUS rc = CameraQueryImage(index_, buffer, &len, opt);
  CameraTriggerShot(index_);
  return API_OK == rc;
}

namespace
{
  struct ThreadParam {
    volatile LONG* async_capturing;
    int index;
    bool use_trigger;
    int buf_len;
    unsigned char* buf;
  };
  DWORD WINAPI CaptureThreadFunc(void* p) {
    ThreadParam* param = (ThreadParam*)p;
    int result = API_OK;
    int opt = CAMERA_IMAGE_RAW8;
    if (param->use_trigger) {
      opt = CAMERA_IMAGE_GRAY8 | CAMERA_IMAGE_TRIG;
    }

    result = CameraQueryImage(param->index, param->buf, &param->buf_len, opt);

    volatile LONG* async_capturing = param->async_capturing;
    delete param;
    InterlockedDecrement(async_capturing);
    return result;
  }
}

HANDLE UsbCamera::CaptureFrameAsync(bool use_trigger,
                                    unsigned char* buffer /* = NULL */) {
  if (async_capturing_ > 0) return NULL;
  if (!buffer_ || !buffer) return NULL;

  InterlockedIncrement(&async_capturing_);

  ThreadParam* param = new ThreadParam;
  param->async_capturing = &async_capturing_;
  param->index = index_;
  param->use_trigger = use_trigger;
  param->buf_len = buffer_length_;
  if (buffer) {
    param->buf = buffer;
  } else {
    param->buf = buffer_;
  }

  return CreateThread(NULL, 0, CaptureThreadFunc, param, 0, NULL);
}

bool UsbCamera::IsAsyncSuccess(HANDLE handle) {
  if (async_capturing_ > 0) return false;

  DWORD exit_code = 1;
  if (GetExitCodeThread(handle, &exit_code)) {
    return exit_code == 0;
  }
  return false;
}