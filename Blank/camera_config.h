#pragma once

#include <string>

namespace base
{
  class DictionaryValue;
}

struct CameraConfig {
  CameraConfig();
  int gain;
  int expo;
  int reso;
  int id;
  std::string sn;
  bool x_revert;
  bool y_revert;

  static bool ParseFromDict(base::DictionaryValue* dict, CameraConfig& config);
};
