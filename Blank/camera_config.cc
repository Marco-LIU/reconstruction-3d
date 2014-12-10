#include "camera_config.h"

#include "base/values.h"
#include "base/logging.h"

CameraConfig::CameraConfig()
    : reso(-1), expo(-1), gain(-1), id(-1), x_revert(false), y_revert(false) {

}

// static
bool CameraConfig::ParseFromDict(base::DictionaryValue* dict,
                                 CameraConfig& config) {
  DCHECK(dict);
  int id = 0;
  std::string sn;
  if (dict->GetInteger("id", &id) &&
      dict->GetStringASCII("sn", &sn)) {
    dict->GetInteger("resolution", &config.reso);
    dict->GetInteger("gain", &config.gain);
    dict->GetInteger("expo", &config.expo);
    dict->GetBoolean("x_revert", &config.x_revert);
    dict->GetBoolean("y_revert", &config.y_revert);
    config.id = id;
    config.sn = sn;
    return true;
  }
  return false;
}